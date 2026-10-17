"""Offline inspection of the pinned consumer v2 coefficient layout.

No Windows audio/registry/native-library access. Default inspection is read-only;
the explicit candidate-directory option writes a separate project-local candidate.
Structural/public-model tests do not establish native acceptance.
"""
from __future__ import annotations

from dataclasses import dataclass
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

HEADER = bytes.fromhex('4100000001000000')
BLOCKS = (
    ('rp_header', 0, 13), ('shared_field_exponent', 13, 14),
    ('room_allpass_delays', 14, 17), ('room_allpass_gains', 17, 20),
    ('room_branch_scalar', 20, 21), ('room_ear_filters', 21, 1045),
    ('room_delay_integers', 1045, 1050), ('room_feedback_filters', 1050, 1562),
    ('room_mixing_matrix', 1562, 1578), ('room_output_taps', 1578, 1834),
    ('directional_pca', 1834, 15624), ('shared_distance_trailer', 15624, 15691),
)

def digest(data):
    return hashlib.sha256(data).hexdigest()

@dataclass(frozen=True)
class Payload:
    booleans: tuple
    integer: int | None
    coefficients: tuple | None
    version: bytes | None

class Reader:
    def __init__(self, data):
        self.data, self.position = data, 0
    def read(self, size):
        if size < 0 or self.position + size > len(self.data):
            raise ValueError('Truncated initialization payload')
        value = self.data[self.position:self.position+size]
        self.position += size
        return value
    def flag(self):
        value = self.read(1)[0]
        if value not in (0, 1):
            raise ValueError('Invalid presence/Boolean value')
        return bool(value)
    def uint(self):
        return struct.unpack('<I', self.read(4))[0]
    def optional_bool(self):
        return self.flag() if self.flag() else None

def inspect_rp(values):
    if len(values) != 15691 or any(type(v) is not int or not -2**31 <= v < 2**31 for v in values):
        raise ValueError('Only the observed complete 15691-lane signed-int32 layout is supported')
    words = [v & 65535 for v in values]
    if words[0] != 0x7072 or words[2:13] != [1,1,0,64,3,0,1,25,20,11,36]:
        raise ValueError('Unsupported rp header/layout')
    extended = 0xa569
    for value in words[2:]:
        extended ^= value
    core = 0xa569
    for value in words[2:-11]:
        core ^= value
    if extended != words[1]:
        raise ValueError('Extended rp checksum mismatch')
    return {'extended_checksum':extended, 'core_checksum':core,
            'extended_valid':True, 'core_valid':core == words[1]}

def decode(data, envelope='api'):
    if envelope == 'registry':
        if data[:8] != HEADER:
            raise ValueError('Invalid registry envelope')
        data = data[8:]
    elif envelope != 'api':
        raise ValueError('Envelope must be api or registry')
    reader = Reader(data)
    b1, b2 = reader.optional_bool(), reader.optional_bool()
    integer = reader.uint() if reader.flag() else None
    if reader.flag():
        raise ValueError('v1 payload is outside this v2-only inspector scope')
    values = version = None
    if reader.flag():
        count = reader.uint()
        if count > 32768:
            raise ValueError('Coefficient count exceeds native bound')
        values = struct.unpack(f'<{count}i', reader.read(count*4))
        size = reader.uint()
        if size > 20:
            raise ValueError('Version exceeds native bound')
        version = reader.read(size)
        inspect_rp(values)
    if reader.flag():
        raise ValueError('v3 payload shadows v2 and is outside inspector scope')
    b3 = reader.optional_bool()
    if reader.position != len(data):
        raise ValueError('Trailing initialization bytes')
    return Payload((b1,b2,b3), integer, values, version)

def room_tap_proposal(values):
    """Return an isolated in-memory copy for inspection or candidate generation."""
    inspect_rp(values)
    proposed = list(values)
    proposed[1578:1834] = [0]*256
    checksum = 0xa569
    for value in proposed[2:]:
        checksum ^= value & 65535
    proposed[1] = checksum
    inspect_rp(proposed)
    return tuple(proposed)

