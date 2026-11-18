# Object-based crosstalk cancellation

Filter the desired ear signals through a measured speaker-to-ear matrix. Two speakers and two ears require four paths measured with shared timing and gain.

## Signal chain

The object renderer first produces desired left/right ear signals from object PCM and a SOFA field. A two-input/two-output speaker filter then compensates a measured speaker-to-ear transfer matrix. For two speakers and two ears, the physical plant has four paths with common timing, gain, ear order and listener pose.

Inverting independently normalized or independently shifted paths solves a different problem. A physically valid measurement should retain relative speaker/ear timing and level. Regularization and cancellation bandwidth address conditioning and excessive gain; they do not make a missing or wrong plant measurement valid.

## Implemented processing

The native partitioned implementation processes 128-frame blocks with prepared filters. It supports object starts/stops, generation-based identity, input-time filter transitions and complete tail retirement. Tests compare the full chain with independent direct convolution rather than only comparing two implementations of the same truncated kernel.

A shared two-stage path uses an object-to-ear convolution stage followed by a 2×2 XTC stage. For static source directions, precombining the filters retains all 8191 combined coefficients in the example and removes one 128-frame buffer. It does not remove the inverse filter's selected target delay.

The sample inverse includes a deliberate 480-frame, 10 ms target delay. At 48 kHz, the fused example's buffer-plus-target-delay total is 12.667 ms before retained SOFA timing and device effects. That is a design example, not a measured acoustic total or a lower bound for every XTC system.

## Motion

Head-relative source rotation can be verified algebraically, but listener movement also changes the physical speaker-to-ear plant. Rotating a virtual source is not the same as updating that plant. Real tracking needs timestamped pose transport, a plant/model valid for the new pose and controlled filter transitions.

The processing tests exercise source motion and filter changes offline. They do not verify tracker latency or robust cancellation over a listening area. The available physical-path inventory contained only synthetic fixtures, so no physical cancellation conclusion was possible.

## Real-time limits

Prepared filters, fixed pools, bounded history and allocation-free tested callbacks are useful constraints. They do not make Windows scheduling deterministic. Paced runs retain both successful cases and deadline/wake outliers. The maximum observed CPU work is not a guarantee under unrelated load.

The next acoustic experiment requires actual four-path data and an explicit level/pose/calibration protocol. Until that input exists, improving a synthetic inverse or reporting its modeled ear error does not establish a working physical system.
