
* WSL 2 is unusably slow if you're keeping your files on the non-virtualized Windows OS. [Microsoft knows this and is looking for solutions, but it's an extremely challenging technical problem.](https://github.com/microsoft/WSL/issues/4197#issuecomment-604592340)
  
  You can downgrade from WSL 2 to WSL 1 by running the `wsl --set-version Ubuntu 1` in a Windows-side shell (i.e. open a new cmd/powershell window).