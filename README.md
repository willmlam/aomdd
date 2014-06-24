# AOMDD Compiler

This code compiles Bayesian networks and Markov networks to AOMDDs and computes P(e) and MPE queries.

### Prerequisites
Boost 1.46 or newer. (not tested with older versions)

### Options
Input files:
  -f [ --file ] arg     path to problem file (UAI format) [required]
  -o [ --order ] arg    path to elimination ordering file [required]
  -e [ --evid ] arg     path to evidence file

Output files:
  -t [ --treedot ] arg  path to DOT file to output generated pseudo-tree
  -r [ --res ] arg      path to output results
  -d [ --dddot ] arg    path to DOT file to output generated AOMDD

Inference options:
  -c [ --compile ]      compile full AOMDD
  --pe                  compute P(e)
  --mpe                 computer MPE(e) cost
  --mbe                 use minibucket approximation
  --mbebound arg        diagram size bound for minibucket
  --mbeibound arg       i-bound for minibucket
  --vbe                 use standard function tables
  --log                 output results in log space

Other options:
  --computeortreesize   compute OR tree size only
  --cmonly              generate cm graph instead(via compilation)
  --chaintree           use chain pseudo-tree structure
  --mlim arg            Memory limit (MB) for nodes
  --oclim arg           Memory limit (MB) for operation cache
  --outcompile          Output compiled AOMDD
  --bespace             Output space for standard table BE (only)
  -h [ --help ]         Output list of options
