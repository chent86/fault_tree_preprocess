# Preprocessing for Fault tree

## Environment
- Ubuntu 18.04
- GNU Make 4.1
- gcc 7.3.0
## Quick start
```
make
./main -INPUT_DIR=example/input -OUTPUT_DIR=example/output
```
## Config

path: include/config.hpp

|Name|Function|Default value|
|-|-|-|
|INPUT_DIR|The path of folder where input files are stored|"example/input"|
|OUTPUT_DIR|The path of folder where output files will be stored|"example/output"|
|SAME_GATE|Simplify when parent node and it's child node have same type of gate event if set true.|true|
|ONE_CHILD|Simplify when a node has only one child if set true.|true|
|SAME_TREE|Simplify when two subtree are equivalent if set true.|true|
|NORMAL_PROCESS|Get models of the tree if set true, else return original tree.|true|
|LCC_PROCESS|Run Local Combination Check when finding models if set true.|true|
|SIMPLE_OUTPUT|Return simple version output if set true.|false|

## Procedure of preprocessing

### Simplification

#### Parameter:SAME_GATE
If parent node r1 has the same type of gate event as it's child node g1, then let node g1's children be node r1's children, and remove node g1 from node r1's children set. For example:
```
r1 := (g1 & g2);
g1 := (e1 & e2);
```
Will be simplified as:
```
r1 := (e1 & e2 & g2);
```
#### Parameter:ONE_CHILD
If parent node r1's child node g1 has only one child e1, then let node e1 be node r1's child, and remove node g1 from node r1's children set. For example:
```
r1 := (g1);
g1 := (e1);
```
Will be simplified as:
```
r1 := (e1);
```
#### Parameter:SAME_TREE
If two subtree are equivalent in the tree, then let the top node of one subtree replace the other top node. For example:
```
r1 := (g1 & g2);
g1 := (e1 | e2);
g2 := (g3 | e3);
g3 := (e1 | e2);
```
Will be simplified as:
```
r1 := (g1 & g2);
g1 := (e1 | e2);
g2 := (g1 | e3);
```
### Finding models
Please refer to: [Kohda, Ernest J Henley, and Koichi Inoue. 1989. Finding Modules
in Fault Trees.]

