#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
#
# generated by wxGlade 1.1.0pre on Tue Oct  4 09:55:21 2022
#

# This is an automatically generated file.
# Manual changes will be overwritten without warning!

import wx
from gs_ref_dlg_GUI import gs_ref_dlg_GUI


class MyApp(wx.App):
    def OnInit(self):
        self.gs_frame = gs_ref_dlg_GUI(None, wx.ID_ANY, "")
        self.SetTopWindow(self.gs_frame)
        self.gs_frame.Show()
        return True

# end of class MyApp

if __name__ == "__main__":
    app = MyApp(0)
    app.MainLoop()
