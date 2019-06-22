//krazy:skip
/**
  \namespace Plugins
  \brief Implementation of the KIPI and Purpose interfaces.

  As of this writing, KIPI is no longer maintained and will probably phase out of distributions soon.
  Purpose, on the other hand, is a replacement for some, but not (yet) all things that KIPI plugins could do.

  ## KIPI Status:
  <ul>
  <li>Last checked against libkipi version 5.1.0 (v16.07.80).</li>
  <li>Implemented features are described by Plugins::Interface::features()</li>
  <li>As far as implemented, everything should work.</li>
  <li>Some concepts, such as image orientation, have no exact match between libkipi and kphotoalbum.</li>
  </ul>

  ## Purpose Status:
   - Sharing images is possible
   - Success/error feedback does not yet work correctly
**/
// vi:expandtab:tabstop=4 shiftwidth=4:
