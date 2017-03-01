/*
  Copyright (C) 2000-2012 Novell, Inc
  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/


/*-/

  File:		YProperty.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YProperty_h
#define YProperty_h

#include <string>
#include <vector>



enum YPropertyType
{
    YUnknownPropertyType = 0,
    YOtherProperty,		// requires futher checking
    YStringProperty,		// const std::string &
    YBoolProperty,		// bool
    YIntegerProperty,		// YCP Integer == C++ long long
    YColorProperty		// YColor
};

class YWidget;
class YProperty;

typedef long long	YInteger;
typedef uint32_t	YColor;

#define YCOLOR_NONE		0xFFFFFFFF
#define YCOLOR_TRANSPARENT	0xFF000000
#define YCOLOR_RED		0x00FF0000
#define YCOLOR_GREEN		0x0000FF00
#define YCOLOR_BLUE		0x000000FF
#define YCOLOR_WHITE		0x00FFFFFF
#define YCOLOR_BLACK		0x00000000

std::string YColorToString( YColor color )
{
    if (color == YCOLOR_NONE)
        return "";
    static const char* digits = "0123456789ABCDEF";
    std::string rc(7, '0');
    rc[0] = '#';
    for (size_t i=1, j=20; i<7; ++i,j-=4)
       rc[i] = digits[(color>>j) & 0xF];
    return rc;
}

/**
 * Class for widget properties.
 **/
class YProperty
{
public:
    /**
     * Constructor: Create a property with the specified name and type.
     * 'isReadOnly' is for properties that cannot be set, only retrieved.
     **/
    YProperty( const std::string & name, YPropertyType type, bool isReadOnly = false )
	: _name( name )
	, _type( type )
	, _isReadOnly( isReadOnly )
	{}

    /**
     * Returns the name of this property.
     **/
    std::string name() const { return _name; }

    /**
     * Returns the type of this property.
     **/
    YPropertyType type() const { return _type; }

    /**
     * Returns 'true' if this property cannot be changed, only retrieved.
     **/
    bool isReadOnly() const { return _isReadOnly; }

    /**
     * Returns the type of this property as string.
     **/
    std::string typeAsStr() const { return YProperty::typeAsStr( _type ); }

    /**
     * Returns a string description of a property type.
     **/
    static std::string typeAsStr( YPropertyType type );

private:

    std::string		_name;
    YPropertyType	_type;
    bool		_isReadOnly;
};


/**
 * Transport class for the value of simple properties.
 *
 * More complex properties (lists of items, tree descriptions, ...) have to
 * be handled specifically someplace else, but most properties are of
 * simple types and can be treated in similar ways.
 **/
class YPropertyValue
{
public:

    /**
     * Constructor for string properties.
     **/
    YPropertyValue( const std::string & str ):
	_type( YStringProperty ), _stringVal( str ) {}

    /**
     * Constructor for const char * (string) properties.
     **/
    YPropertyValue( const char * str ):
	_type( YStringProperty ), _stringVal( str ) {}

    /**
     * Constructor for bool properties.
     **/
    explicit YPropertyValue( bool b ):
	_type( YBoolProperty ), _boolVal( b ) {}

    /**
     * Constructor for numerical (YCP integer) properties.
     **/
    explicit YPropertyValue( YInteger num ):
	_type( YIntegerProperty ), _integerVal( num ) {}

    /**
     * Constructor for numerical (YCP integer) properties.
     **/
    explicit YPropertyValue( int num ):
	_type( YIntegerProperty ), _integerVal( num ) {}

    /**
     * Constructor for color (YColor) properties.
     **/
    explicit YPropertyValue( YColor color ):
	_type( YColorProperty ), _colorVal( YColor ) {}

    explicit YPropertyValue( YPropertyType type ) :
	_type( type ) {}

    /**
     * Default constructor
     **/
    YPropertyValue():
	_type( YUnknownPropertyType ) {}

    /**
     * Destructor.
     **/
    ~YPropertyValue();

    /**
     * Equality operator, can compare with another YPropertyValue.
     * @throw YUIException for incompatible property types
     * @return true if the value is the same
     */
    bool operator==( const YPropertyValue &other ) const;

    /** Inequality operator
     * @throw YUIException for incompatible property types
     * @see operator==
     */
    bool operator!=( const YPropertyValue &other ) const;

    /**
     * Returns the type of this property value.
     * Use this to determine which xyVal() method to use.
     **/
    YPropertyType type() const { return _type; }

    /**
     * Returns the type of this property value as string.
     **/
    std::string typeAsStr() const { return YProperty::typeAsStr( _type ); }

    /**
     * Methods to get the value of this property.
     * Check with type() which one to use.
     **/
    std::string	stringVal() 	const { return _stringVal;	}
    bool 	boolVal()	const { return _boolVal;	}
    YInteger 	integerVal()	const { return _integerVal;	}
    YColor 	colorVal()	const { return _colorVal;	}


private:

    YPropertyType	_type;
    std::string		_stringVal;
    bool		_boolVal;
    YInteger		_integerVal;
    YColor		_colorVal;
};


/**
 * A set of properties to check names and types against.
 **/
class YPropertySet
{
public:
    /**
     * Constructor.
     **/
    YPropertySet();

    /**
     * Check if a property 'propertyName' exists in this property set.
     * Throw a YUIUnknownPropertyException if it does not exist.
     * Use YPropertySet::contains() for a check that simply returns 'false'
     * if it does not exist.
     **/
    void check( const std::string & propertyName ) const;

    /**
     * Check if a property 'propertyName' exists in this property set.
     * Throw a YUIUnknownPropertyException if it does not exist.
     *
     * If there is a property with that name, check also the expected type
     * against 'type'. If the types don't match, throw a
     * YUIPropertyTypeMismatchException.
     * If the property is read-only, throw a YUISetReadOnlyPropertyException.
     **/
    void check( const std::string & propertyName, YPropertyType type ) const;

    /**
     * Same as above, overloaded for convenience.
     **/
    void check( const YProperty & prop ) const
	{ check( prop.name(), prop.type() ); }

    /**
     * Check if a property 'propertyName' exists in this property set.
     * Returns 'true' if it exists, 'false' if not.
     *
     * Use YPropertySet::check() for a check that throws exceptions if
     * there is no such property.
     **/
    bool contains( const std::string & propertyName ) const throw();

    /**
     * Check if a property 'propertyName' exists in this property set.
     * Returns 'true' if it exists, 'false' if not.
     *
     * If there is a property with that name, check also the expected type
     * against 'type'. If the types don't match, throw a
     * YUIPropertyTypeMismatchException.
     *
     * If the property is read-only, throw a YUISetReadOnlyPropertyException.
     *
     * Use YPropertySet::check() for a check that throws exceptions if
     * there is no such property.
     **/
    bool contains( const std::string & propertyName, YPropertyType type ) const;

    /**
     * Same as above, overloaded for convenience.
     **/
    bool contains( const YProperty & prop ) const
	{ return contains( prop.name(), prop.type() ); }

    /**
     * Returns 'true' if this property set does not contain anything.
     **/
    bool isEmpty() const { return _properties.empty(); }

    /**
     * Returns the number of properties in this set.
     **/
    int size() const { return (int) _properties.size(); }

    /**
     * Add a property to this property set.
     **/
    void add( const YProperty & prop );

    /**
     * Adds all properties of another property set.
     *
     * If that other set contains duplicates (properties that are already
     * in this set), those others will never be found with lookup().
     **/
    void add( const YPropertySet & otherSet );

    typedef std::vector<YProperty>::const_iterator const_iterator;

    /**
     * Returns an iterator that points to the first property in this set.
     **/
    const_iterator propertiesBegin() const;

    /**
     * Returns an iterator that points after the last property in this set.
     **/
    const_iterator propertiesEnd() const;

private:

    /**
     * This class uses a simple std::vector as a container to hold the
     * properties: Normally, the number of properties for each widget is so
     * small (2..5) that using any more sophisticated container like
     * std::set etc. would not pay off. More likely, it would add overhead.
     **/
    std::vector<YProperty> _properties;
};


#endif // YProperty_h
