# Project:   Widget5

.SUFFIXES:    .c .o
CC            = cc
CCFlags       = -c -fussy -throwback -IC: -Ff
LinkFlags     = -o $@
SqueezeFlags  = -f -v

Files         = o.Widget5 o.SpriteWin o.Create o.Misc o.LoadSave o.ATrans o.CheckBanks o.ProcessWin o.Configure o.Process o.Filter o.GraphWin o.MixWin o.GreyEdWin

Libs          = Hacklib:o.HackLib Desklib:o.DeskLib C:o.Stubs C:flexlib.o


!RunImage:    $(Files) 
              link $(LinkFlags) $(Files) $(Libs)
              squeeze $(SqueezeFlags) !RunImage

.c.o:;        $(CC) $(CCflags) $< -o $@
