# A.I. solver for Zen Garden game
A.I. solver using **Evolution algorithm**

![mid-game state](http://www2.fiit.stuba.sk/~kapustik/zen-s.png)
## Installation
on linux:
```
gcc -o zen-solver main.c
```
## Usage
Prepare config file:
* first line: map dimensions [y,x]
* other lines: rock position [y,x]

#### Example of config file
```
10 12
1 5
2 1
3 4
4 2
6 8
6 9
```
#### Generated map from input file (for visualization)
```
0 0 0 0 0 0 0 0 0 0 0 0 
0 0 0 0 0 1 0 0 0 0 0 0 
0 2 0 0 0 0 0 0 0 0 0 0 
0 0 0 0 3 0 0 0 0 0 0 0 
0 0 4 0 0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 5 6 0 0 
0 0 0 0 0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 0 0 0 0 
```
#### Execution
```
./zen-solver <map config file> <max. number of generations>
./zen-solver map.conf 10000
```

#### Program output
```
gen0, best fitness: 87
gen100, best fitness: 104
gen200, best fitness: 108
gen300, best fitness: 108
gen400, best fitness: 104
gen500, best fitness: 104
gen600, best fitness: 104
gen700, best fitness: 104
gen800, best fitness: 104
gen900, best fitness: 108
```

## Implementation
   A unit is represented by two types of genes:  
   * Start positions on map border
   * Direction change upon rock collision
   
   First generation is randomly generated  
   Every new generation is made of:  
   * 1/2 of units from previous generation picked by tournament selection (1/913 to pick worse one)
   * 1/4 is mutated gene by gene (1/3 to pick a gene from worse one) from tournament picked units
   * 1/4 is newly generated  