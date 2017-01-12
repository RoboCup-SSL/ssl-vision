//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    qgetopt.h
  \brief   command line option parser
  \author  froglogic Porten & Stadlbauer GbR, (C) 2003, 2004
*/
//========================================================================

#ifndef GETOPT_H
#define GETOPT_H

#include <QString>
#include <QStringList>
#include <QLinkedList>
#include <QMap>

class GetOpt {
public:
    GetOpt();
    GetOpt( int offset );
    GetOpt( int argc, char *argv[] );
    GetOpt( const QStringList &a );

    QString appName() const { return aname; }

    // switch (no arguments)
    void addSwitch( const QString &lname, bool *b );
    void addOptSwitch( const QString &lname, bool *b, bool def=false);
    void addShortSwitch( char s, const QString &lname, bool *b );
    void addShortOptSwitch( char s, const QString &lname, bool *b, bool def=false);

    // options (with arguments, sometimes optional)
    void addOption( char s, const QString &l, QString *v );
    void addVarLengthOption( const QString &l, QStringList *v );
    void addRepeatableOption( char s, QStringList *v );
    void addRepeatableOption( const QString &l, QStringList *v );
    void addOptionalOption( const QString &l, QString *v,
                                const QString &def );
    void addOptionalOption( char s, const QString &l,
				QString *v, const QString &def );

    // bare arguments
    void addArgument( const QString &name, QString *v );
    void addOptionalArgument( const QString &name, QString *v );

    bool parse( bool untilFirstSwitchOnly );
    bool parse() { return parse( false ); }

    bool isSet( const QString &name ) const;

    int currentArgument() const { return currArg; }

private:
    enum OptionType { OUnknown, OEnd, OSwitch, OArg1, OOpt, ORepeat, OVarLen, OOptSwitch, OShortSwitch, OShortOptSwitch };

    struct Option;
    friend struct Option;

    struct Option {
        Option( OptionType t = OUnknown,
                char s = 0, const QString &l = QString::null )
            : type( t ),
              sname( s ),
              lname( l ),
              boolValue( 0 ) { }

        OptionType type;
        char sname;		// short option name (0 if none)
        QString lname;	// long option name  (null if none)
        union {
            bool *boolValue;
            QString *stringValue;
            QStringList *listValue;
        };
        QString def;
    };

    QLinkedList<Option> options;
    typedef QLinkedList<Option>::const_iterator OptionConstIterator;
    QMap<QString, int> setOptions;

    void init( int argc, char *argv[], int offset = 1 );
    void addOption( Option o );
    void setSwitch( const Option &o );

    QStringList args;
    QString aname;

    int numReqArgs;
    int numOptArgs;
    Option reqArg;
    Option optArg;

    int currArg;
};

#endif

