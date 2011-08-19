# AOMDD Compiler

This code compiles Bayesian networks and Markov networks to AOMDDs and computes P(e) and MPE queries.

### Prerequisites
Boost

### Options
    Input files:
      -f <file>       path to problem file (UAI format) [required]
      -o <file>       path to elimination ordering file [required]
      -e <file>       path to evidence file

    Output files:
      -t <file>       path to DOT file to output generated pseudo-tree
      -r <file>       path to output results

    Inference options:
      -c              compile full AOMDD first
      -pe             compute P(e)
      -mpe            compute MPE(e) cost
      -vbe            use vanilla bucket elimination
      -log            output results in log space
