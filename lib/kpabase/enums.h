/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KPABASE_ENUMS_H
#define KPABASE_ENUMS_H

namespace AnnotationDialog
{
enum UsageMode { InputSingleImageConfigMode,
                 InputMultiImageConfigMode,
                 SearchMode };
enum MatchType { MatchFromBeginning,
                 MatchFromWordStart,
                 MatchAnywhere };
/** @short Distinguishes between user-induced changes to ResizableFrame and automatic ones.
 *
 * Manual changes are any ones that are in some way conscious by the user (e.g. setting the tag data).
 * Automatic changes suppress some signals (e.g. when setting a transient value).
 */
enum ChangeOrigin { ManualChange,
                    AutomatedChange };
}
enum ListSelectEditMode {
    ReadOnly, ///< values can not be changed
    Selectable, ///< existing values can be (de)selected, but no value can be added or removed
    Editable ///< values can be freely added, selected, and removed
};

#endif /* KPABASE_ENUMS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
