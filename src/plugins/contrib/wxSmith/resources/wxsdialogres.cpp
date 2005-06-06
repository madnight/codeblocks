#include "wxsdialogres.h"

#include "../wxswidgetfactory.h"
#include "../wxswindoweditor.h"
#include <manager.h>
#include <wx/xrc/xmlres.h>
#include <editormanager.h>


const char* EmptySource =
"\
#include \"$(Include)\"\n\
\n\
//(*EventTable($(ClassName))\n\
BEGIN_EVENT_TABLE($(ClassName),wxDialog)\n\
END_EVENT_TABLE()\n\
//*)\n\
\n\
$(ClassName)::$(ClassName)(wxWidnow* parent,wxWindowID id = -1)\n\
{\n\
    //(*Initialize($(ClassName))\n\
    //*)\n\
}\n\
\n\
$(ClassName)::~$(ClassName)()\n\
{\n\
}\n\
\n\
";

const char* EmptyHeader =
"\
#ifndef __$(Guard)\n\
#define __$(Guard)\n\
\n\
//(*Headers($(ClassName))\n\
//*)\n\
\n\
class $(ClassName): public wxDialog\n\
{\n\
    public:\n\
\n\
        $(ClassName)(wxWidnow* parent,wxWindowID id = -1);\n\
        virtual ~$(ClassName);\n\
\n\
    protected:\n\
\n\
        //(*Handlers\n\
        //*)\n\
        //(*Declarations($(ClassName))\n\
        //*)\n\
\n\
    private:\n\
\n\
        DECLARE_EVENT_TABLE()\n\
};\n\
\n\
#endif\n\
";

wxsDialogRes::wxsDialogRes(wxsProject* Project,const wxString& Class, const wxString& Xrc, const wxString& Src,const wxString& Head):
    wxsResource(Project),
    ClassName(Class),
    XrcFile(Xrc),
    SrcFile(Src),
    HFile(Head)
{
    Dialog = dynamic_cast<wxsDialog*>(wxsWidgetFactory::Get()->Generate("wxDialog"));
}

wxsDialogRes::~wxsDialogRes()
{
    EditClose();
    wxsWidgetFactory::Get()->Kill(Dialog);
    GetProject()->DeleteDialog(this);
}

wxsEditor* wxsDialogRes::CreateEditor()
{
    wxsWindowEditor* Edit = new wxsWindowEditor(Manager::Get()->GetEditorManager()->GetNotebook() ,XrcFile,this);
    Edit->BuildPreview(Dialog);
    return Edit;
}

void wxsDialogRes::Save()
{
    TiXmlDocument* Doc = GenerateXml();
    
    if ( Doc )
    {
        wxString FullFileName = GetProject()->GetInternalFileName(XrcFile);
        Doc->SaveFile(FullFileName);
        delete Doc;
    }
}

TiXmlDocument* wxsDialogRes::GenerateXml()
{
    TiXmlDocument* NewDoc = new TiXmlDocument;
    TiXmlElement* Resource = NewDoc->InsertEndChild(TiXmlElement("resource"))->ToElement();
    TiXmlElement* XmlDialog = Resource->InsertEndChild(TiXmlElement("object"))->ToElement();
    XmlDialog->SetAttribute("class","wxDialog");
    XmlDialog->SetAttribute("name",ClassName.c_str());
    if ( !Dialog->XmlSave(XmlDialog) )
    {
        delete NewDoc;
        return NULL;
    }
    return NewDoc;
}

void wxsDialogRes::ShowPreview()
{
    Save();
    
    wxXmlResource Res(GetProject()->GetInternalFileName(XrcFile));
    Res.InitAllHandlers();
    
    wxDialog* Dlg = Res.LoadDialog(NULL,ClassName);
    if ( Dlg )
    {
        Dlg->ShowModal();
        delete Dlg;
    }
}

const wxString& wxsDialogRes::GetResourceName()
{
    return GetClassName();
}

bool wxsDialogRes::GenerateEmptySources()
{
    // Generating file variables
    
    wxString FName = wxFileName(HFile).GetName();
    FName.MakeUpper();
    wxString Guard(wxT("__"));
    
    for ( int i=0; i<(int)FName.Length(); i++ )
    {
        char ch = FName.GetChar(i);
        if ( ch < 'A' || ch > 'Z' ) Guard.Append('_');
        else Guard.Append(ch);
    }
    
    wxFileName IncludeFN(GetProject()->GetProjectFileName(HFile));
    IncludeFN.MakeRelativeTo(GetProject()->GetProjectFileName(SrcFile));
    wxString Include = IncludeFN.GetFullPath();
    

    FILE* Fl = fopen(GetProject()->GetProjectFileName(HFile),"wt");
    if ( !Fl ) return false;
    wxString Content = EmptyHeader;
    Content.Replace("$(Guard)",Guard,true);
    Content.Replace("$(ClassName)",ClassName,true);
    fprintf(Fl,"%s",Content.c_str());
    fclose(Fl);
    
    Fl = fopen(GetProject()->GetProjectFileName(SrcFile),"wt");
    if ( !Fl ) return false;
    Content = EmptySource;
    Content.Replace("$(Include)",Include,true);
    Content.Replace("$(ClassName)",ClassName,true);
    fprintf(Fl,"%s",Content.c_str());
    fclose(Fl);
    return true;
}
