## Unordered bidirectional map implementation

Unordered bimap is a date structure that contains set of key-value pairs
and provides constant time complexity for either key and value lookup.

### How does it work.

There's simply two hash tables used for both keys and values.

Notice that key-value node is allocated and stored once.

Added elements are linked in the same order they are added into a bimap.
