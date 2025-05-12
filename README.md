# SplineCC: A Cubic Spline-Based Congestion Control Algorithm

## Overview
SplineCC is a custom congestion control algorithm implemented in C (`SpCC.c`) designed to optimize network performance by dynamically adjusting the congestion window (`cwnd`) based on Round-Trip Time (RTT), throughput, and the number of acknowledgments (ACKs). Unlike traditional algorithms like TCP Reno or CUBIC, SplineCC uses cubic splines to achieve smoother and more adaptive window adjustments, making it suitable for networks with variable latency and bandwidth.

This algorithm is ideal for developers and researchers working on network protocols, particularly in environments with high latency, fluctuating throughput, or specific performance requirements.

## Features
- **Cubic Spline Adaptation**: Uses coefficients (`c`, `d`, `b`) derived from RTT, `cwnd`, and throughput to smoothly adjust the congestion window.
- **Slow Start Phase**: Implements an aggressive window increase during the initial phase, transitioning to congestion avoidance when the threshold (`ssthresh`) is reached.
- **Congestion Detection**: Responds to network congestion (e.g., increased RTT or reduced ACKs) by conservatively reducing the window.
- **Optimized Computations**: Employs fast arithmetic macros (e.g., `MULu64_FAST`, `DIV3`) and caching to minimize computational overhead.
- **Duplicate ACK Handling**: Reduces the window and threshold in response to packet loss indicators.

## Computations
The `SplineCC` function computes the next congestion window (`next_cwnd`) based on input parameters: `curr_cwnd`, `curr_rtt`, `throughput`, `num_acks`, and a state structure (`sCC`). Below is a breakdown of the key computational steps, illustrated with an example where:
- `curr_cwnd = 20` (segments), `curr_rtt = 150` (µs), `throughput = 2000` (bytes/s), `num_acks = 3`
- Initial state: `last_rtt = 100` (µs), `last_cwnd = 15`, `ssthresh = 30`, `last_max_cwnd = 25`, `last_min_rtt = 100`, `last_ack = 2`

### Step-by-Step Computations
1. **Initialization**:
   - Set `state->curr_cwnd = 20` and `state->curr_rtt = 150`.

2. **Coefficient `d` (RTT-based, `find_cof_rtt`)**:
   - Compute the ratio of current to last RTT: `(curr_rtt * 8) / last_rtt = (150 * 8) / 100 = 12`.
   - Calculate cubic ratio: `(12 * 12 * 12) / 2 = 864`.
   - Since `curr_rtt > last_rtt + ERR_R` (150 > 100 + 5), compute `loc_rtt = 864 + (curr_rtt / 2) = 864 + 75 = 939`.
   - Compute `result = loc_rtt + (loc_rtt / 2) = 939 + 469 = 1408`.
   - Set `d = (ratio * 2) + ((result + loc_rtt) / loc_rtt) = (12 * 2) + (1408 + 939) / 939 ≈ 24 + 2 = 26`.
   - Update `state->d = 26`, `state->d_initial = 26`, `state->last_rtt = 150`.

3. **Slow Start (`handle_slow_start`)**:
   - Check if `curr_cwnd < ssthresh`: `20 < 30`, so slow start is active.
   - Update ACKs: `state->last_ack = 2`, `state->curr_ack = 3`.
   - Detect congestion: `curr_rtt = 150 > last_min_rtt * 39 / 32 ≈ 121.875`, so use conservative growth.
   - Increase window: `curr_cwnd += (last_cwnd / 8) + ((num_acks + 1) / 2) = 20 + (15 / 8) + ((3 + 1) / 2) = 20 + 1 + 2 = 23`.
   - Update `state->next_cwnd = 23`, `state->last_cwnd = 23`.
   - Since `slow_start_cwnd = 23`, return this value and skip further steps.

4. **Threshold Adjustment (`ssthresh_comp`)**:
   - Compute `max_ssthresh = DIV100(last_max_cwnd * THRESHOLD_PERCENT) = (25 * 100 * 3) / 256 ≈ 29`.
   - Since `curr_ack = 3 > last_ack = 2` and `ssthresh = 30 > max_ssthresh = 29`, set `ssthresh = 29`.

5. **Other Coefficients (if slow start is inactive)**:
   - **Coefficient `c` (`find_cof_cwnd`)**: If `curr_cwnd >= ssthresh`, compare `last_cwnd` and `curr_cwnd` to compute `c`. For example, if `curr_cwnd = 30`, `c = (last_cwnd + curr_cwnd) - d_initial = (18 + 30) - 26 = 22`.
   - **Coefficient `b` (`find_cof_bw`)**: Compute `throughput / (c + d_initial) = 2000 / (22 + 26) ≈ 41`, then `b = throughput_t / throughput ≈ 48`.

6. **Next Window (`resolve_next_cwnd`)**:
   - If slow start is inactive, adjust `next_cwnd` based on network conditions:
     - Stable network: Increase `cwnd` by `d` and ACK growth.
     - Congestion: Reduce `cwnd` by 10% (using `MULT0_9`).
     - Limit `next_cwnd` to twice `last_max_cwnd`.

### Example Output
For the given inputs, `SplineCC` returns `next_cwnd = 23` (slow start phase) and updates `ssthresh = 29`.

### Key Aspects
- **Cubic Splines**: Nonlinear adjustments via `ratio_cubed` ensure smooth adaptation.
- **Fast Arithmetic**: Macros like `MULu64_FAST` and `DIVu64` optimize performance.
- **Caching**: Stores `cached_ratio` and `cached_throughput` to reduce redundant calculations.
- **Constraints**: Limits `cwnd`, RTT, and `ssthresh` to prevent overflow or instability.

## Installation
To use SplineCC, you need a C compiler (e.g., `gcc`) and a basic development environment.

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/splinecc.git
   cd splinecc
   ```
2. Compile the code:
   ```bash
   gcc -o splinecc SpCC.c
   ```
   Note: You may need to integrate `SpCC.c` into a larger networking application or simulation framework (e.g., `ns-3` or a custom TCP stack).

## Usage
To integrate SplineCC into your project:
1. Include `SpCC.c` in your codebase.
2. Initialize an `sCC` structure to maintain state:
   ```c
   sCC state = {0}; // Initialize all fields to zero
   ```
3. Call the `SplineCC` function in your packet processing loop:
   ```c
   u32 next_cwnd = SplineCC(curr_cwnd, curr_rtt, throughput, num_acks, &state);
   ```
4. Use the returned `next_cwnd` to set the congestion window for your protocol.

### Example
```c
#include "SpCC.c"
#include <stdio.h>

int main() {
    sCC state = {0};
    u32 curr_cwnd = 10;    // Initial window (segments)
    u32 curr_rtt = 100;    // RTT in microseconds
    u64 throughput = 1000; // Throughput in bytes/second
    u32 num_acks = 2;      // Number of ACKs

    u32 next_cwnd = SplineCC(curr_cwnd, curr_rtt, throughput, num_acks, &state);
    printf("Next congestion window: %u segments\n", next_cwnd);

    return 0;
}
```

## Configuration
The algorithm uses several hardcoded constants that can be tuned for specific networks:
- `MAX_RTT`: Maximum RTT (1,000,000 µs = 1 second).
- `MIN_RTT`: Minimum RTT (10 µs).
- `ERR_R`: RTT error tolerance (5 µs).
- `MAX_SSHTHRESH`: Maximum slow-start threshold (1000 segments).
- `THRESHOLD_PERCENT`: Percentage for computing `ssthresh` (100%).

To modify these, edit the `#define` directives at the top of `SpCC.c`. Future versions may allow dynamic configuration via the `sCC` structure.

## Testing
To validate SplineCC:
1. **Simulation**: Use network simulators like `ns-3` or `Mininet` to test performance under varying conditions (e.g., high latency, packet loss).
2. **Real-World Testing**: Integrate with a TCP stack or custom protocol and monitor throughput, latency, and packet loss.
3. **Edge Cases**: Test with extreme values (e.g., `curr_rtt = MIN_RTT` or `MAX_RTT`, zero ACKs).

## Limitations
- **Complexity**: The use of cubic splines and multiple coefficients makes the algorithm more complex than TCP Reno or CUBIC.
- **Loss Handling**: Limited support for advanced packet loss recovery (e.g., Selective ACKs or retransmission timeouts).
- **32-bit Constraints**: Uses 32-bit integers for `cwnd` and RTT, which may limit scalability in ultra-high-speed networks.
- **Fixed Parameters**: Hardcoded constants may not be optimal for all network types (e.g., low-latency LANs vs. high-latency WANs).

## Future Improvements
- Add dynamic parameter tuning based on network conditions.
- Enhance packet loss recovery with support for Selective ACKs (SACK) or retransmission timeouts (RTO).
- Transition to 64-bit integers for `cwnd` and RTT to support high-speed networks.
- Simplify coefficient calculations for better maintainability.
- Integrate pacing to distribute packet sending evenly over time.
