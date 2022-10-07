from .gs_ref_dlg_GUI import *
from .gscholar import *

class GSRefDlg(gs_ref_dlg_GUI):
    
    def __init__(self,parent=None,id=-1,title="Google Scholar Dialog"):
        parent = wx.FindWindowByName("Bibliographic Database")
        gs_ref_dlg_GUI.__init__(self,parent,id,title)
        self.gs_m = GScholar()
        pass
    
    def OnTest1(self,event):
        print("test1")

    def get_refs_on_page(self, event):  
        self.gs_m.find_info_from_refs() 
        event.Skip()