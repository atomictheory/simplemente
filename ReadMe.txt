Simplemente is a Winboard compatible atomic chess engine.

To set it up in Winboard in the winboard.ini file include the following:

/firstChessProgramNames={
"simplemente" -fd "[directory where simplemente.exe is located]" /variant=atomic
}

Load it with Engine -> Load first engine and select 'simplemente (atomic)'.

To compile simplemente as Winboard engine XBOARD_COMPATIBLE has to be defined in xboard.h.

If XBOARD_COMPATIBLE is not defined ( commented out ) in xboard.h, then it compiles as a deep analysis book builder in command interpreter mode. The compiled executable of this version is 'simplebook.exe'. For instructions in simplebook type help+ENTER.

