from .gs_ref_dlg_GUI import *
from .gscholar import *

class GSRefDlg(gs_ref_dlg_GUI):
    
    def __init__(self,parent=None,id=-1,title="Google Scholar Dialog", use_user_data = False, gs_m = None ):
        parent = wx.FindWindowByName("Bibliographic Database")
        gs_ref_dlg_GUI.__init__(self,parent,id,title)
        if( gs_m ):
            self.gs_m = gs_m
        else:    
            self.gs_m = GScholar(use_user_data = use_user_data )
    
    def OnTest1(self,event):
        print("test1")

    def get_refs_on_page(self, event):  
        self.gs_m.find_info_from_refs() 
        event.Skip()