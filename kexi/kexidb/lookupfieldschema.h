/* This file is part of the KDE project
   Copyright (C) 2006 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KEXIDB_LOOKUPFIELDSCHEMA_H
#define KEXIDB_LOOKUPFIELDSCHEMA_H

#include <qvaluelist.h>
#include <qstringlist.h>

class QDomElement;
class QDomDocument;

namespace KexiDB {

//! default value for LookupFieldSchema::columnHeadersVisible()
#define KEXIDB_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE false

//! default value for LookupFieldSchema::maximumListRows()
#define KEXIDB_LOOKUP_FIELD_DEFAULT_LIST_ROWS 8

//! maximum value for LookupFieldSchema::maximumListRows()
#define KEXIDB_LOOKUP_FIELD_MAX_LIST_ROWS 100

//! default value for LookupFieldSchema::limitToList()
#define KEXIDB_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST true

//! default value for LookupFieldSchema::displayWidget()
#define KEXIDB_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET KexiDB::LookupFieldSchema::ComboBox


//! @short Provides information about lookup field's setup.
/*! 
 LookupFieldSchema object is owned by TableSchema and created upon creating or retrieving the table schema
 from the database metadata.

 @see LookupFieldSchema *TableSchema::lookupFieldSchema( Field& field ) const
*/
class KEXI_DB_EXPORT LookupFieldSchema
{
	public:

		//! Row source information that can be specified for the lookup field schema
		class KEXI_DB_EXPORT RowSource {
			public:
				//! Row source type
				enum Type {
					NoType,         //!< used for invalid schema
					Table,        //!< table as lookup row source
					Query,        //!< named query as lookup row source
					SQLStatement, //!< anonymous query as lookup row source
					ValueList,    //!< a fixed list of values as lookup row source
					FieldList     //!< a list of column names from a table/query will be displayed
				};

				RowSource();
				~RowSource();

				/*! @return row source type: table, query, anonymous; in the future it will 
				 be also fixed value list and field list. The latter is basically a list 
				 of column names of a table/query, "Field List" in MSA. */
				Type type() const { return m_type; }

				/*! Sets row source type to \a type. */
				void setType(Type type) { m_type = type; }

				/*! @return row source type name. @see setTypeByName() */
				QString typeName() const;

				/*! Sets row source type by name using \a typeName. Accepted (cast sensitive) 
				 names are "table", "query", "sql", "valuelist", "fieldlist".
				 For other value NoType type is set. */
				void setTypeByName( const QString& typeName );

				/*! @return a string for row source: table name, query name or anonymous query 
				 provided as KEXISQL string. If rowSourceType() is a ValueList, 
				 rowSourceValues() should be used instead. If rowSourceType() is a FieldList,
				 rowSource() should return table or query name. */
				QString name() const { return m_name; }

				/*! Sets row source value. @see value() */
				void setName(const QString& name);

				/*! @return row source values specified if type() is ValueList. */
				QStringList values() const;

				/*! Sets row source values used if type() is ValueList. 
				 Using it clears name (see name()). */
				void setValues(const QStringList& values);

				/*! \return String for debugging purposes. */
				QString debugString() const;

				/*! Shows debug information. */ 
				void debug() const;
			private:
				Type m_type;
				QString m_name;
				QStringList *m_values;
		};

		LookupFieldSchema();

		~LookupFieldSchema();

		/*! @return row source information for the lookup field schema */
		RowSource& rowSource() { return m_rowSource; }

		/*! Sets row source for the lookup field schema */
		void setRowSource(const RowSource& rowSource) { m_rowSource = rowSource; }

		/*! @return bound column: an integer specifying a column that is bound 
		 (counted from 0). -1 means unspecified value. */
//! @todo in later implementation there can be more columns
		int boundColumn() const { return m_boundColumn; }

		/*! Sets bound column number to \a column. @see boundColumn() */
		void setBoundColumn(int column) { m_boundColumn = column>=0 ? column : -1; }

		/*! @return visible column: an integer specifying a column that has 
		 to be visible in the combo box (counted from 0). 
		 -1 means unspecified value. */
//! @todo in later implementation there can be more columns
		int visibleColumn() const { return m_visibleColumn; }

		/*! Sets visible column number to \a column. @see visibleColumn() */
		void setVisibleColumn(int column) { m_visibleColumn = column>=0 ? column : -1; }

		/*! @return a number of ordered integers specifying column widths;
		 -1 means 'default width' for a given column. */
		const QValueList<int> columnWidths() const { return m_columnWidths; }

		/*! Sets column widths. @see columnWidths */
		void setColumnWidths(const QValueList<int>& widths) { m_columnWidths = widths; }

		/*! @return true if column headers are visible in the associated 
		 combo box popup or the list view. The default is false. */
		bool columnHeadersVisible() const { return m_columnHeadersVisible; }

		/*! Sets "column headers visibility" flag. @see columnHeadersVisible() */
		void setColumnHeadersVisible(bool set) { m_columnHeadersVisible = set; }

		/*! @return integer property specifying a maximum number of rows 
		 that can be displayed in a combo box popup or a list box. The default is 
		 equal to KEXIDB_LOOKUP_FIELD_DEFAULT_LIST_ROWS constant. */
		uint maximumListRows() const { return m_maximumListRows; }

		/*! Sets maximum number of rows that can be displayed in a combo box popup 
		 or a list box. If \a rows is 0, KEXIDB_LOOKUP_FIELD_DEFAULT_LIST_ROWS is set.
		 If \a rows is greater than KEXIDB_LOOKUP_FIELD_MAX_LIST_ROWS, 
		 KEXIDB_LOOKUP_FIELD_MAX_LIST_ROWS is set. */
		void setMaximumListRows(uint rows);

		/*! @return true if , only values present on the list can be selected using
		 the combo box. The default is true. */
		bool limitToList() const { return m_limitToList; }

		/*! Sets "limit to list" flag. @see limitToList() */
		void setLimitToList(bool set) { m_limitToList = set; }

		//! used in displayWidget()
		enum DisplayWidget {
			ComboBox, //!< (the default) combobox widget should be displayed in forms for this lookup field
			ListBox   //!< listbox widget should be displayed in forms for this lookup field
		};

		/*! @return the widget type that should be displayed within 
		 the forms for this lookup field. The default is ComboBox. 
		 For the Table View, combo box is always displayed. */
		DisplayWidget displayWidget() const { return m_displayWidget; }

		/*! Sets type of widget to display within the forms for this lookup field. @see displayWidget() */
		void setDisplayWidget(DisplayWidget widget) { m_displayWidget = widget; }

		/*! \return String for debugging purposes. */
		QString debugString() const;

		/*! Shows debug information. */ 
		void debug() const;

		/*! Loads data of lookup column schema from DOM tree. 
		 The data can be outdated or invalid, so the app should handle such cases.
		 @return a new LookupFieldSchema object even if lookupEl contains no valid contents. */
		static LookupFieldSchema* loadFromDom(const QDomElement& lookupEl);

		/*! Saves data of lookup column schema to \a parentEl DOM element of \a doc document. */
		static void saveToDom(LookupFieldSchema& lookupSchema, QDomDocument& doc, QDomElement& parentEl);

	protected:
		RowSource m_rowSource;
		int m_boundColumn, m_visibleColumn;
		QValueList<int> m_columnWidths;
		uint m_maximumListRows;
		DisplayWidget m_displayWidget;
		bool m_columnHeadersVisible : 1;
		bool m_limitToList : 1;
};

} //namespace KexiDB

#endif
