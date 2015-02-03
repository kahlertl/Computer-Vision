# Lab 5: Stereo matching with Markov chains and dynamic programming

Implement a stereo reconstruction algorithm using dynamic programming on a tree.

 * Use this simple tree construction
   ```
   ○--○--○--○--○
         |
   ○--○--○--○--○
         |
   ○--○--○--○--○
         |
   ○--○--○--○--○
         |
   ○--○--○--○--○
   ```
 * Disparity is in the range (0...15).

### Matching cost q_i

- use difference of colors in 5x5 window.
- Michael Bleyer, Sylvie Chambon. "Does Color Really Help in Dense Stereo Matching ?". 3DPVT, 2010.)

### Smoothness cost g(d,d’) :

- G*(d != d') (potts model)
- G*|d-d'|
- G*(d-d')²

Where G is a constant. 


## Build

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# generate Makefile
$ cmake .

# compile binary
$ make

# execute
$ ./optical_flow <frame1> <frame2>
```