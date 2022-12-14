#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
#
# generated by wxGlade 1.1.0pre on Mon Oct  3 18:33:21 2022
#

import wx

# begin wxGlade: dependencies
# end wxGlade

# begin wxGlade: extracode
# end wxGlade


class gs_ref_dlg(wx.Frame):
    def __init__(self, *args, **kwds):
        # begin wxGlade: gs_ref_dlg.__init__
        kwds["style"] = kwds.get("style", 0) | wx.DEFAULT_FRAME_STYLE
        wx.Frame.__init__(self, *args, **kwds)
        self.SetSize((400, 300))
        self.SetTitle("frame")

        self.panel_1 = wx.Panel(self, wx.ID_ANY)

        sizer_1 = wx.BoxSizer(wx.VERTICAL)

        self.button_1 = wx.Button(self.panel_1, wx.ID_ANY, "button_1")
        sizer_1.Add(self.button_1, 0, 0, 0)

        self.button_2 = wx.Button(self.panel_1, wx.ID_ANY, "button_2")
        sizer_1.Add(self.button_2, 0, 0, 0)

        sizer_2 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_1.Add(sizer_2, 1, wx.EXPAND, 0)

        self.button_3 = wx.Button(self.panel_1, wx.ID_ANY, "button_3")
        sizer_2.Add(self.button_3, 0, 0, 0)

        self.button_4 = wx.Button(self.panel_1, wx.ID_ANY, "button_4")
        sizer_2.Add(self.button_4, 0, 0, 0)

        self.panel_1.SetSizer(sizer_1)

        self.Layout()
        # end wxGlade

# end of class gs_ref_dlg

class MyApp(wx.App):
    def OnInit(self):
        self.frame = gs_ref_dlg(None, wx.ID_ANY, "")
        self.SetTopWindow(self.frame)
        self.frame.Show()
        return True

# end of class MyApp

if __name__ == "__main__":
    app = MyApp(0)
    app.MainLoop()
