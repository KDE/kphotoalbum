/* Define to 1 if Marble should be compiled in */
#cmakedefine HAVE_MARBLE 1
#ifdef HAVE_MARBLE
#cmakedefine MARBLE_HAS_regionSelected_OLD
#cmakedefine MARBLE_HAS_regionSelected_NEW

#if !(defined(MARBLE_HAS_regionSelected_OLD) || defined(MARBLE_HAS_regionSelected_NEW))
#error "Feature detection for Marble failed!"
#endif
#endif
