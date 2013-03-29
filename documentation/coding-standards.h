/**
   \page coding-standards Coding Standards

   Here are a few pointers regarding coding standards in
   KPhotoAlbum. Please do stick to them, so the code is easier to get into
   for new people, and easier to maintain for everyone.

   \li Indentation is 4 spaces
   \li Instance variables are prefixed either with an underscore or
   m_. Please stick to one of the two for a given class and use the m_ variant
   for new classes.
   \li Methods that are overridden from a superclass should be marked as such
   using the C++11 override keyword.
   \li KPhotoAlbum is warning free zone. Please keep it that way. No warnings during compilations are accepted.
   \li Keep debugging messages out of production code. If you really have to
   push debug statements to git master, add the commit id to the respective
   bit report so it can be reverted once the bug is resolved. If there is no
   fitting bug report (e.g. for new code), disable the debug statements by
   default.<br/>
   You can use this recipe for adding debug statements to your code:<br>
   \code
   #ifdef DEBUG_FOO
   #define Debug qDebug
   #else
   #define Debug if (0) qDebug
   #endif
   ...
   Debug() << "This will only be printed out if DEBUG_FOO is defined.";
   \endcode
   You can then enable the code by adding DEBUG_FOO to the CMAKE_CXX_FLAGS
   variable in your cmake cache.

   <h2>Include files and forward declarations</h2>
   To speed up compilation and make things easier to understand, you should
   be careful about what you include, and when cleaning up code, please
   check whether you need all the include files.

   In header files you should try hard to see if you really need an include
   file at all, or whether you can get by with only a forward declaration.

   A forward declaration looks like:
   \code
     class MyClass;
     namespace MyNameSpace { class MyClass; }
   \endcode

   You can get by with only a forward declaration when all you do is one of
   these:
   \li declare a method that passes a pointer or reference as argument
   (doSomething( const MyClass& )
   \li return an object from a method (MyClass getClass() )
   \li you only have the class in a container (QList<MyClass>)

   In contrast you do need the include files when:
   \li you declare a method that takes a value argument (doSomething(
   MyClass cls) )
   \li you have an instance variable of the given class (MyClass _class).

*/
// vi:expandtab:tabstop=4 shiftwidth=4:
