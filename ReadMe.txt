simplemente is a Winboard compatible atomic chess engine

to set it up in Winboard in the winboard.ini file include the following:

/firstChessProgramNames={
"simplemente" -fd "[directory where simplemente.exe is located]" /variant=atomic
}

load it with Engine -> Load first engine and select 'simplemente (atomic)'