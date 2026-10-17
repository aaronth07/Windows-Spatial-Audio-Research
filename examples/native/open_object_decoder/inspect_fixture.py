"""Read an MP4 audio sample entry and first compressed bytes without decoding."""
import argparse
import hashlib
import json
from pathlib import Path
import struct


def uhd_prefix(packet):
    if packet[:4]!=bytes.fromhex('40411bf2'):raise ValueError('Not a UHD sync frame')
    bits=''.join(format(byte,'08b') for byte in packet);cursor=32;index=0
    def bit():
        nonlocal cursor
        if cursor>=len(bits):raise ValueError('Truncated UHD header')
        value=bits[cursor];cursor+=1;return value
    while index<3 and bit()=='1':index+=1
    widths=[5,8,10,12];width=widths[index]
    value=int(''.join(bit() for _ in range(width)),2)
    length=sum(1<<n for n in widths[:index])+value+1
    full=bit()=='1'
    if length>len(packet) or length*8<cursor:raise ValueError('Invalid FTOC extent')
    return {'ftoc_header_bytes':length,'full_channel_based_mix_flag':full,
            'basis':'ETSI TS103491 sections5.2.3.1 and6.4.3; header prefix only, CRC not recomputed.'}


def boxes(data,start,end):
    while start<end:
        if end-start<8:raise ValueError('Truncated MP4 box')
        size,kind=struct.unpack_from('>I4s',data,start);header=8
        if size==1:
            if end-start<16:raise ValueError('Truncated large box')
            size=struct.unpack_from('>Q',data,start+8)[0];header=16
        elif size==0:size=end-start
        if size<header or start+size>end:raise ValueError('Invalid MP4 box extent')
        yield kind,start+header,start+size
        start+=size


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('media',type=Path);parser.add_argument('output',type=Path)
    args=parser.parse_args()
    if args.output.exists():raise FileExistsError(args.output)
    data=args.media.read_bytes();tracks=[]
    def children(kind,start,end):return [(a,b) for k,a,b in boxes(data,start,end) if k==kind]
    for ma,mb in children(b'moov',0,len(data)):
        for ta,tb in children(b'trak',ma,mb):
            for da,db in children(b'mdia',ta,tb):
                handlers=children(b'hdlr',da,db)
                if not handlers or data[handlers[0][0]+8:handlers[0][0]+12]!=b'soun':continue
                for ia,ib in children(b'minf',da,db):
                    for sa,sb in children(b'stbl',ia,ib):
                        ds=children(b'stsd',sa,sb);offsets=children(b'stco',sa,sb);wide=False
                        if not offsets:offsets=children(b'co64',sa,sb);wide=True
                        sizes=children(b'stsz',sa,sb)
                        if not ds or not offsets or not sizes:raise ValueError('Unsupported sample table')
                        a=offsets[0][0];count=struct.unpack_from('>I',data,a+4)[0]
                        if not count:raise ValueError('Empty chunk table')
                        offset=struct.unpack_from('>Q' if wide else '>I',data,a+8)[0]
                        a=sizes[0][0];fixed,samples=struct.unpack_from('>II',data,a+4)
                        size=fixed or struct.unpack_from('>I',data,a+12)[0]
                        if offset+size>len(data):raise ValueError('Sample outside MP4')
                        tag=data[ds[0][0]+12:ds[0][0]+16].decode('ascii')
                        packet=data[offset:offset+size];sync=packet[:4].hex()
                        header={}
                        if sync=='40411bf2':
                            header=uhd_prefix(packet)
                        tracks.append({'sample_entry':tag,'sample_count':samples,'first_sample_offset':offset,'first_sample_bytes':size,'first_32_bytes_hex':packet[:32].hex(),
                                       'sync_word_hex':sync,'matches_dts_uhd_sync':sync in ['40411bf2','71c442e8'],
                                       'matches_dca_core_sync':sync=='7ffe8001','matches_exss_sync':sync=='64582025','parsed_uhd_header':header})
    if not tracks:raise ValueError('No supported MP4 audio track')
    result={'media_sha256':hashlib.sha256(data).hexdigest(),'audio_tracks':tracks,
            'scope':'MP4 sample-table and sync-word observation only; full profile syntax/CRC not parsed.'}
    args.output.write_text(json.dumps(result,indent=2));print(json.dumps(result))


if __name__=='__main__':main()
