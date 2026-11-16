# First-pass reflection


# Testing
## Tests that could still be written
### Scan
- With a max batch size
- With a large table
### Filter
- Other types of Compare operations
- 50% pass filter
- 75% pass filter
- 100% pass filter
### Pipeline
- Large table

# Hash Join Debugging
Produce:
1. The first probe row with an incorrect output count.
- 49. 
2. The expected and actual pair counts for that row.
- expected: 21, actual: 5
3. My explanation of which invariant is being violated, or, if I can't determine it yet, which experiment I would run next to distinguish between competing hypotheses.