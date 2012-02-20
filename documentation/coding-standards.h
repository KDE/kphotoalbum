/**
   \page coding-standards Coding Standards

   Here are a few pointers regarding coding standards in
   KPhotoAlbum. Please do stick to them, so the code is easier to get into
   for new people, and easier to maintain for everyone.

   \li Indentation is 4 spaces
   \li Instance variables are prefixed either with an underscore or
   m_. Please stick to one of the two for a given class.
   \li Methods that are overridden from a superclass should be marked with
   the macro OVERRIDE. This macro expands to nothing, so it is purely for
   bringing to peoples attention that this is actually overriding a method.
   \li KPhotoAlbum is warning free zone. Please keep it that way. No warnings during compilations are accepted.

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
