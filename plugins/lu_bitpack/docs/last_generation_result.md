
# `last_generation_result`

A singleton which holds information about the last successful code generation operation. This exists to serve pragmas such as `serialized_offset_to_constant`.

When a code generation operation finishes, this singleton stores a copy of the per-sector serialization item lists used for codegen. When it's asked for information about some serialized value, it'll lazy-create fully-expanded serialization item lists in order to facilitate searching for the value in question.