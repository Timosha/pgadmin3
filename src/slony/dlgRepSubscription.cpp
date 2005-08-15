//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2005, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// dlgRepSubscription.cpp - PostgreSQL Slony-I Subscription Property
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"

#include "dlgRepSubscription.h"
#include "slCluster.h"
#include "slSubscription.h"
#include "slSet.h"



// pointer to controls
#define txtOrigin       CTRL_TEXT("txtOrigin")
#define txtReceiver     CTRL_TEXT("txtReceiver")
#define cbProvider      CTRL_COMBOBOX("cbProvider")
#define chkForward      CTRL_CHECKBOX("chkForward")



BEGIN_EVENT_TABLE(dlgRepSubscription, dlgProperty)
    EVT_COMBOBOX(XRCID("cbProvider"),   dlgRepSubscription::OnChange)
    EVT_CHECKBOX(XRCID("chkForward"),   dlgRepSubscription::OnChange)
END_EVENT_TABLE();


dlgProperty *slSubscriptionFactory::CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
{
    return new dlgRepSubscription(frame, (slSubscription*)node, (slSet*)parent);
}

dlgRepSubscription::dlgRepSubscription(frmMain *frame, slSubscription *sub, slSet *s)
: dlgRepProperty(frame, s->GetCluster(), wxT("dlgRepSubscription"))
{
    subscription=sub;
    set=s;
}


pgObject *dlgRepSubscription::GetObject()
{
    return subscription;
}


int dlgRepSubscription::Go(bool modal)
{
    txtOrigin->SetValue(NumToStr(subscription->GetSet()->GetOriginId()));
    if (subscription)
    {
        // edit mode
        chkForward->SetValue(subscription->GetForward());
        txtReceiver->SetValue(IdAndName(subscription->GetReceiverId(), subscription->GetReceiverNode()));
    }
    else
    {
        // create mode
        txtReceiver->SetValue(IdAndName(cluster->GetLocalNodeID(), cluster->GetLocalNodeName()));
    }

    if (set->GetOriginId() == cluster->GetLocalNodeID() && subscription)
    {
        chkForward->SetValue(subscription->GetForward());
        cbProvider->Append(IdAndName(subscription->GetProviderId(), subscription->GetProviderNode()));
        cbProvider->SetSelection(0);
        cbProvider->Disable();
        chkForward->Disable();
        EnableOK(false);
    }
    else
    {
        pgSet *sets = connection->ExecuteSet(
            wxT("SELECT no_id, no_comment\n")
            wxT("  FROM ") + cluster->GetSchemaPrefix() + wxT("sl_node\n") 
            wxT(" WHERE no_id <> ") + NumToStr(cluster->GetLocalNodeID()));

        if (sets)
        {
            while (!sets->Eof())
            {
                cbProvider->Append(IdAndName(sets->GetLong(wxT("no_id")), sets->GetVal(wxT("no_comment"))),
                    (void*)sets->GetLong(wxT("no_id")));

                if (subscription && sets->GetLong(wxT("no_id")) == subscription->GetProviderId())
                    cbProvider->SetSelection(cbProvider->GetCount()-1);
                sets->MoveNext();
            }
            delete sets;
        }
    }
    if (!subscription && cbProvider->GetCount())
    {
        cbProvider->SetSelection(0);
        EnableOK(true);
    }
    return dlgProperty::Go(modal);
}


pgObject *dlgRepSubscription::CreateObject(pgCollection *collection)
{
    pgObject *obj=subscriptionFactory.CreateObjects(collection, 0,
         wxT(" WHERE set_id = ") + NumToStr(set->GetSlId()) +
         wxT("   AND sub_receiver = ") + NumToStr(cluster->GetLocalNodeID()));

    return obj;
}


void dlgRepSubscription::CheckChange()
{
    if (subscription)
    {
        int sel = cbProvider->GetSelection();

        EnableOK(sel >= 0 && (chkForward->GetValue() != subscription->GetForward()
            ||   (long)cbProvider->GetClientData(sel) != subscription->GetProviderId()));
    }
    else
    {
        bool enable=true;

        EnableOK(enable);
    }
}



wxString dlgRepSubscription::GetSql()
{
    wxString sql;

    sql = wxT("SELECT ") + cluster->GetSchemaPrefix() + wxT("subscribeset(")
        + NumToStr(set->GetSlId()) + wxT(", ")
        + NumToStr((long)cbProvider->GetClientData(cbProvider->GetSelection())) + wxT(", ")
        + NumToStr(cluster->GetLocalNodeID()) + wxT(", ")
        + BoolToStr(chkForward->GetValue()) + wxT(");");

    return sql;
}
