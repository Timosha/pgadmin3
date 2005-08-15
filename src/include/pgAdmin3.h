//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2005, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// pgAdmin3.h - The main application header
//
//////////////////////////////////////////////////////////////////////////

#ifndef PGADMIN3_H
#define PGADMIN3_H

// wxWindows headers
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/xrc/xmlres.h>

#include "misc.h"
#include <ctl/ctlTree.h>
#include "ctl/ctlSQLBox.h"
#include "ctl/ctlListView.h"
#include "ctl/ctlComboBox.h"
#include "dlgClasses.h"
#include "pgConn.h"
#include "pgSet.h"
#include "base/factory.h"

#include "precomp.h"

// App headers
#include "sysSettings.h"

#ifdef __WXMSW__
#else
#include "config.h"
#undef VERSION
#endif


const float SERVER_MIN_VERSION = 7.3f;

#ifdef __WIN32__
#define SPLASH_FONTSIZE 8
#else
#if wxCHECK_VERSION(2,5,0)
#define SPLASH_FONTSIZE 9
#else
#define SPLASH_FONTSIZE 11
#endif
#endif

#define SPLASH_X0       6
#define SPLASH_Y0       114
#define SPLASH_OFFS     15


class pgAppearanceFactory
{
public:
    pgAppearanceFactory();

    void SetIcons(wxDialog *dlg);
    void SetIcons(wxTopLevelWindow *dlg);
    char **GetSmallIconImage();
    char **GetBigIconImage();
    char **GetSplashImage();
};

extern pgAppearanceFactory *appearanceFactory;


#endif // PGADMIN3_H
