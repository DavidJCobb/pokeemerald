
# `assert-monkeypatch`

Modifies `console.assert` to actually halt execution (as best as is possible) by throwing an error after logging. This is not foolproof &mdash; any caller can potentially catch and swallow the thrown error (the Python problem) &mdash; but as far as I know, it's the best that the language allows.

See [here](https://gist.github.com/DavidJCobb/97bcbb0b23ec75779f585faa4cbfff71) for further information.

