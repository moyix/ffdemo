Combinations of different flags (out of 100, 3 trials):

```
DEFAULT 86 84 85 
NOYIELD 26 43 39 
PIN_THREAD 90 90 86 
USE_RDTSC 61 64 62 
NO_DOUBLE_ACCESS 94 84 84 
NOYIELD+PIN_THREAD 34 42 48 
NOYIELD+USE_RDTSC 41 52 49 
NOYIELD+NO_DOUBLE_ACCESS 47 44 41 
PIN_THREAD+USE_RDTSC 58 64 59 
PIN_THREAD+NO_DOUBLE_ACCESS 98 84 87 
USE_RDTSC+NO_DOUBLE_ACCESS 58 60 49 
NOYIELD+PIN_THREAD+USE_RDTSC 59 57 54 
NOYIELD+PIN_THREAD+NO_DOUBLE_ACCESS 58 48 51 
NOYIELD+USE_RDTSC+NO_DOUBLE_ACCESS 48 55 59 
PIN_THREAD+USE_RDTSC+NO_DOUBLE_ACCESS 53 61 59 
NOYIELD+PIN_THREAD+USE_RDTSC+NO_DOUBLE_ACCESS 53 50 49 
```

Number of iterations on the `NO_DOUBLE_ACCESS` setting which seemed to be the most reliable combination:

```
8 41 50 54 
16 51 50 57 
32 57 53 52 
64 55 62 52 
128 60 62 59 
256 60 55 52 
512 65 65 51 
1024 52 66 65 
2048 68 59 55 
4096 64 65 72 
8192 72 63 73 
16384 80 66 73 
32768 80 72 79 
65536 69 76 76 
131072 67 74 75 
262144 75 76 81 
524288 73 74 77 
1048576 76 83 76 
2097152 85 81 78 
4194304 83 82 81 
8388608 85 90 86 
16777216 87 90 90 
33554432 90 89 88 
```
