# Deraen's i3-utils

Some i3 utils which should work with minimal dependencies.

Dependencies:

- Glib 2.0 (should be present everywhere)
- Glib-json 1.0

## Why

Existing utilities often use Python bindings or such. I didn't like them. Two
approaches I like:

- Bash scripts, however e.g. parsing JSON in Bash is not very easy (though it is possible)
- C programs, Glib is widely available and seems to be useful for parsing JSON and such
