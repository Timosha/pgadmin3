//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// pgColumn.cpp - Column class
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "misc.h"
#include "pgDefs.h"
#include "pgDatatype.h"

#include "pgObject.h"
#include "pgColumn.h"
#include "pgCollection.h"


pgColumn::pgColumn(pgSchema *newSchema, const wxString& newName)
: pgSchemaObject(newSchema, PG_COLUMN, newName)
{
    isFK=false;
    isPK=false;
}

pgColumn::~pgColumn()
{
}


bool pgColumn::DropObject(wxFrame *frame, wxTreeCtrl *browser)
{
    wxString sql = wxT("ALTER TABLE ") + GetQuotedFullTable();
             sql += wxT(" DROP COLUMN ") + GetQuotedIdentifier();
    
    return GetDatabase()->ExecuteVoid(sql);
}


wxString pgColumn::GetSql(wxTreeCtrl *browser)
{
    if (sql.IsNull() && !GetSystemObject())
    {
        if (GetInheritedCount())
            sql = wxT("-- Column inherited; cannot be changed");
        else
        {
            sql = wxT("-- Column: ") + GetQuotedFullIdentifier() + wxT("\n\n")
                + wxT("-- ALTER TABLE ") + GetQuotedFullTable()
                + wxT(" DROP COLUMN ") + GetQuotedIdentifier()
                
                + wxT("\n\nALTER TABLE ") + GetQuotedFullTable()
                + wxT(" ADD COLUMN ") + GetQuotedIdentifier() + wxT(" ") + GetQuotedTypename()
                
                + wxT(";\nALTER TABLE ")+ GetQuotedFullTable()
                + wxT(" ALTER COLUMN ") + GetQuotedIdentifier()
                + wxT(" SET STORAGE ") + GetStorage() + wxT(";\n");

            if (GetNotNull())
                sql += wxT("ALTER TABLE ") + GetQuotedFullTable()
                    + wxT(" ALTER COLUMN ") + GetQuotedIdentifier()
                    + wxT(" SET NOT NULL;\n");
            if (!GetDefault().IsEmpty())
                sql += wxT("ALTER TABLE ") + GetQuotedFullTable()
                    + wxT(" ALTER COLUMN ") + GetQuotedIdentifier()
                    + wxT(" SET DEFAULT ") + GetDefault() + wxT(";\n");
            if (!GetComment().IsEmpty())
                sql += wxT("COMMENT ON COLUMN ") + GetQuotedFullTable() + wxT(".") + GetQuotedIdentifier()
                    +  wxT(" IS ") + qtString(GetComment()) + wxT(";\n");
        }
    }

    return sql;
}


wxString pgColumn::GetDefinition()
{
    wxString sql = GetQuotedTypename();

    if ((sql == wxT("int4") || sql == wxT("int8"))
        && GetDefault() == wxT("nextval('") 
                        + schema->GetName() + wxT(".") + GetTableName() 
                        + wxT("_") + GetName() + wxT("_seq'::text)"))
    {
        if (sql == wxT("int8"))
            sql = wxT("bigserial");
        else
            sql = wxT("serial");

        if (GetNotNull())
            sql += wxT(" NOT NULL");
    }
    else
    {
        if (GetNotNull())
            sql += wxT(" NOT NULL");
        AppendIfFilled(sql, wxT(" DEFAULT "), GetDefault());
    }
    return sql;
}


void pgColumn::ShowTreeDetail(wxTreeCtrl *browser, frmMain *form, wxListCtrl *properties, wxListCtrl *statistics, ctlSQLBox *sqlPane)
{
    if (!expandedKids)
    {
        expandedKids = true;
        // append type here
        // fk, pk lesen
        pgSet *set = ExecuteSet(
            wxT("SELECT indkey FROM pg_index\n")
            wxT(" WHERE indrelid=") + GetTableOidStr());
        if (set)
        {
            wxString indkey, str;
            while (!isPK && !set->Eof())
            {
                wxStringTokenizer indkey(set->GetVal(0));
                while (indkey.HasMoreTokens())
                {
                    str=indkey.GetNextToken();
                    if (StrToLong(str) == GetColNumber())
                    {
                        isPK = true;
                        break;
                    }
                }

                set->MoveNext();
            }
            delete set;
        }

        set=ExecuteSet(
            wxT("SELECT conkey\n")
            wxT("  FROM pg_constraint ct\n")
            wxT("  JOIN pg_class cl on cl.oid=confrelid\n")
            wxT(" WHERE contype='f' AND conrelid = ") + GetTableOidStr() + wxT("\n")
            wxT(" ORDER BY conname"));
        if (set)
        {
            wxString conkey, str;
            while (!isFK && !set->Eof())
            {
                wxStringTokenizer conkey(set->GetVal(0));

                while (conkey.HasMoreTokens())
                {
                    str=conkey.GetNextToken();
                    if (StrToLong(str.Mid(1)) == GetColNumber())
                    {
                        isFK = true;
                        break;
                    }
                }

                set->MoveNext();
            }
            delete set;
        }
    }

    if (properties)
    {
        CreateListColumns(properties);
        int pos=0;

        InsertListItem(properties, pos++, _("Name"), GetName());
        InsertListItem(properties, pos++, _("Position"), GetColNumber());
        InsertListItem(properties, pos++, _("Data type"), GetVarTypename());
        InsertListItem(properties, pos++, _("Default"), GetDefault());
        InsertListItem(properties, pos++, _("Not NULL?"), GetNotNull());
        InsertListItem(properties, pos++, _("Primary key?"), GetIsPK());
        InsertListItem(properties, pos++, _("Foreign key?"), GetIsFK());
        InsertListItem(properties, pos++, _("Storage"), GetStorage());
        InsertListItem(properties, pos++, _("Inherited"), GetInheritedCount() != 0);

        InsertListItem(properties, pos++, _("System column?"), GetSystemObject());
        InsertListItem(properties, pos++, _("Comment"), GetComment());
    }

    // statistic 
    DisplayStatistics(statistics,
        wxT("SELECT null_frac AS ") + qtIdent(_("Null Fraction")) +
             wxT(", avg_width AS ") + qtIdent(_("Average Width")) +
             wxT(", n_distinct AS ") + qtIdent(_("Distinct Values")) +
             wxT(", most_common_vals AS ") + qtIdent(_("Most Common Values")) +
             wxT(", most_common_freqs AS ") + qtIdent(_("Most Common Frequencies")) +
             wxT(", histogram_bounds AS ") + qtIdent(_("Histogram Bounds")) +
             wxT(", correlation AS ") + qtIdent(_("Correlation")) + wxT("\n")
        wxT("  FROM pg_stats\n")
        wxT(" WHERE schemaname = ") + qtString(schema->GetName()) + wxT("\n")
        wxT("   AND tablename = ") + qtString(GetTableName()) + wxT("\n")
        wxT("   AND attname = ") + qtString(GetName()));
}



pgObject *pgColumn::Refresh(wxTreeCtrl *browser, const wxTreeItemId item)
{
    pgObject *column=0;
    wxTreeItemId parentItem=browser->GetItemParent(item);
    if (parentItem)
    {
        pgObject *obj=(pgObject*)browser->GetItemData(parentItem);
        if (obj->GetType() == PG_COLUMNS)
            column = ReadObjects((pgCollection*)obj, 0, wxT("\n   AND attnum=") + NumToStr(GetColNumber()));
    }
    return column;
}



pgObject *pgColumn::ReadObjects(pgCollection *collection, wxTreeCtrl *browser, const wxString &restriction)
{
    pgColumn *column=0;

    wxString systemRestriction;
    if (!settings->GetShowSystemObjects())
        systemRestriction = wxT("\n   AND attnum > 0");
        
    pgSet *columns= collection->GetDatabase()->ExecuteSet(
        wxT("SELECT att.*, def.*, CASE WHEN attndims > 0 THEN 1 ELSE 0 END AS isarray, CASE WHEN ty.typname = 'bpchar' THEN 'char' WHEN ty.typname = '_bpchar' THEN '_char' ELSE ty.typname END AS typname, tn.nspname as typnspname, et.typname as elemtypname, relname, na.nspname, description\n")
        wxT("  FROM pg_attribute att\n")
        wxT("  JOIN pg_type ty ON ty.oid=atttypid\n")
        wxT("  JOIN pg_namespace tn ON tn.oid=ty.typnamespace\n")
        wxT("  JOIN pg_class cl ON cl.oid=attrelid\n")
        wxT("  JOIN pg_namespace na ON na.oid=cl.relnamespace\n")
        wxT("  LEFT OUTER JOIN pg_type et ON et.oid=ty.typelem\n")
        wxT("  LEFT OUTER JOIN pg_attrdef def ON adrelid=attrelid AND adnum=attnum\n")
        wxT("  LEFT OUTER JOIN pg_description des ON des.objoid=attrelid AND des.objsubid=attnum\n")
        wxT(" WHERE attrelid = ") + collection->GetOidStr()
        + restriction + systemRestriction + wxT("\n")
        wxT("   AND attisdropped IS FALSE\n")
        wxT(" ORDER BY attnum"));

    if (columns)
    {
        while (!columns->Eof())
        {
            column = new pgColumn(collection->GetSchema(), columns->GetVal(wxT("attname")));

            column->iSetTableOid(collection->GetOid());
            column->iSetAttTypId(columns->GetOid(wxT("atttypid")));
            column->iSetColNumber(columns->GetLong(wxT("attnum")));
            column->iSetIsArray(columns->GetBool(wxT("isarray")));
            column->iSetComment(columns->GetVal(wxT("description")));
            if (columns->GetBool(wxT("atthasdef")))
                column->iSetDefault(columns->GetVal(wxT("adsrc")));
            column->iSetStatistics(columns->GetLong(wxT("attstattarget")));

            wxString storage=columns->GetVal(wxT("attstorage"));
            column->iSetStorage(
                storage == wxT("p") ? wxT("PLAIN") :
                storage == wxT("e") ? wxT("EXTERNAL") :
                storage == wxT("m") ? wxT("MAIN") :
                storage == wxT("x") ? wxT("EXTENDED") : wxT("Unknown"));

            column->iSetTyplen(columns->GetLong(wxT("attlen")));

            long typmod=columns->GetLong(wxT("atttypmod"));
            pgDatatype dt(columns->GetVal(wxT("typname")), columns->GetBool(wxT("isarray"))? 1 : 0, typmod);


            column->iSetTypmod(typmod);
            column->iSetLength(dt.Length());
            column->iSetPrecision(dt.Precision());
            column->iSetRawTypename(dt.Name());

            wxString nsp=columns->GetVal(wxT("typnspname"));
            if (nsp == wxT("pg_catalog"))
            {
                column->iSetVarTypename(dt.FullName());
                column->iSetQuotedTypename(dt.QuotedFullName());
            }
            else
            {
                column->iSetVarTypename(nsp + wxT(".") + dt.FullName());
                column->iSetQuotedTypename(qtIdent(nsp) + wxT(".") + dt.QuotedFullName());
            }

            column->iSetNotNull(columns->GetBool(wxT("attnotnull")));
            column->iSetQuotedFullTable(qtIdent(columns->GetVal(wxT("nspname"))) + wxT(".")
                + qtIdent(columns->GetVal(wxT("relname"))));
            column->iSetTableName(columns->GetVal(wxT("relname")));
            column->iSetInheritedCount(columns->GetLong(wxT("attinhcount")));

            if (browser)
            {
                collection->AppendBrowserItem(browser, column);
				columns->MoveNext();
            }
            else
                break;
        }

		delete columns;
    }
    return column;
}
