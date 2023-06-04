# ppty
Pager capable alternative to unbuffer

# Synopsis
`ppty` executes commands in a pty.
This is useful for keeping colors in shell pipes.
In contrast to `unbuffer`, `ppty` supports to pipe coloured output
into a pager like `less`.

# Examples

```bash
# keep highliting from ag
./ppty ag write | less

# keep colored output from `bat` and further colorization from `ag`
# and finally see the colored result in less
./ppty bat --paging=never * | ./ppty ag write | less
```

