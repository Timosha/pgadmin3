//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2007, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// frmPgpassConfig.h - Pgpass.conf editor
//
//////////////////////////////////////////////////////////////////////////

#ifndef frmPgpassConfig_H
#define frmPgpassConfig_H

#include "frmConfig.h"
#include "pgconfig.h"

class pgConn;
class pgServer;
class ctlListView;


WX_DECLARE_OBJARRAY(pgPassConfigLine, pgPassConfigLineArray);

class frmPgpassConfig : public frmConfig
{
public:
    frmPgpassConfig(const wxString& title, const wxString &configFile);
	frmPgpassConfig(frmMain *parent);
    ~frmPgpassConfig();

protected:
    void DisplayFile(const wxString &str);
    void WriteFile(pgConn *conn=0);
    wxString GetHintString();
    wxString GetHelpPage() const;

private:
    void Init();
    void UpdateDisplay(pgPassConfigLine &line);

    void OnContents(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
    void OnEditSetting(wxListEvent& event);
    void OnSelectSetting(wxListEvent& event);

    ctlListView *listEdit;
    pgPassConfigLineArray lines;


    DECLARE_EVENT_TABLE()
};

class pgpassConfigFileFactory : public actionFactory
{
public:
    pgpassConfigFileFactory(menuFactoryList *list, wxMenu *mnu, wxToolBar *toolbar);
    wxWindow *StartDialog(frmMain *form, pgObject *obj);
};

#endif