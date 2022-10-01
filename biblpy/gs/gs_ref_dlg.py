import wx

class GSRefDlg(wx.Frame):
    def __init__(self,parent,id,title):
        wx.Frame.__init__(self, parent, id, title, wx.DefaultPosition, wx.Size(300, 150))

        panel = wx.Panel(self,-1)

        self.btn1 = wx.Button(self,id = wx.ID_ANY, label = "test1")
        self.btn1.Bind(wx.EVT_BUTTON, self.OnTest1)
    
    def OnTest1(self,event):
        print("test1")
