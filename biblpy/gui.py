"""
Functions to start BIBLDB GUI.
"""

def gui():
    """
    Start BIBL DB
    """
    import wx
    import biblpy

    app = wx.App()
    biblpy.StartBiblDBApp()
#    biblpy.StartMainFrame()
    
    app.MainLoop()

