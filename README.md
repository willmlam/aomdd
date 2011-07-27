AOMDD Compiler

This code compiles Bayesian networks and Markov networks to AOMDDs and computes P(e) and MPE queries.

# Options
    -f <file>       path to problem file (UAI format) [required]
    -o <file>       path to elimination ordering file [required]
    -e <file>       path to evidence file
    -t <file>       path to DOT file to output generated pseudo-tree
    -c              compile full AOMDD
    -pe             compute P(e)
    -mpe            compute MPE
    -vbe            use vanilla bucket elimination
    -log            output results in log space
    -verify         verify compiled diagram
