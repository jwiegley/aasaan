import sys
import os

from wxPython.wx import *

if not wxUSE_UNICODE:
    print "Sorry, wxPython was not built with Unicode support.",
    sys.exit(1)

ID_ABOUT = 101
ID_EXIT  = 102

class MyFrame(wxFrame):
    def __init__(self, parent, id, title):
        wxFrame.__init__(self, parent, -1, title, size = (200, 100),
                         style= (wxDEFAULT_FRAME_STYLE |
                                 wxNO_FULL_REPAINT_ON_RESIZE))
        menu = wxMenu()
        menu.Append(ID_ABOUT, "&About",
                    "More information about this program")
        menu.AppendSeparator()
        menu.Append(ID_EXIT, "E&xit", "Terminate the program")

        menuBar = wxMenuBar()
        menuBar.Append(menu, "&File");

        self.SetMenuBar(menuBar)

        self.control = wxTextCtrl(self, -1, style = wxTE_MULTILINE)
        self.Show(true)

        EVT_MENU(self, ID_ABOUT, self.OnAbout)
        EVT_MENU(self, ID_EXIT,  self.TimeToQuit)

    def OnAbout(self, event):
        dlg = wxMessageDialog(self, "This sample program shows off\n"
                              "frames, menus, statusbars, and this\n"
                              "message dialog.",
                              "About Me", wxOK | wxICON_INFORMATION)
        dlg.ShowModal()
        dlg.Destroy()


    def TimeToQuit(self, event):
        self.Close(true)

class MyApp(wxApp):
    def OnInit(self):
        frame = MyFrame(None, -1, "Lughat 1.0")
        self.SetTopWindow(frame)
        return true

app = MyApp(0)
app.MainLoop()
