# Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>
# Copyright (C) 2014 Johannes Zarl <johannes@zarl.at>
# Copyright (C) 2015 Tobias Leupold <tobias.leupold@web.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program (see the file COPYING); if not, write to the
# Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301 USA.

"""
Module for accessing KPhotoAlbum index.xml.
"""

import sys
from xml.dom import minidom
from time import mktime, strptime
from datetime import datetime
from subprocess import check_output
from locale import getpreferredencoding

from KPhotoAlbum.db import DatabaseReader
from KPhotoAlbum.datatypes import Category, MediaItem, Tag, BlockItem, MemberGroup

# Structure of KPhotoAlbum index.xml:
#
# KPhotoAlbum
#     Categories
#         Category
#             value
#                 birthDate
#     images
#        image
#            options
#                option
#                    value
#                    area
#    member-groups
#        member
#    blocklist
#        block

class XMLDatabase(DatabaseReader):
    """
    Class for reading KPhotoAlbum index.xml.
    """

    def __init__(self, filename = None):
        """
        Initialize self with given XML file.

        Pre:
         - ``filename`` is a KPhotoAlbum XML database (index.xml). If no filename is given, the
                        index.xml from the configuration is used.
        """

        super().__init__()

        # If no filename has been given, fetch it from the rc file.

        if filename == None:
            encoding = getpreferredencoding()
            try:
                filename = check_output(['kde4-config', '--localprefix']).decode(encoding).strip()
                filename = filename + 'share/config/kphotoalbumrc'
            except:
                raise ConfigError('Could not fetch local KDE configuration prefix.')

            try:
                # We have to use a rather hackish grep approach here, as kphotoalbumrc does not
                # follow "the rules" of a clean ini file and thus can't be parsed by configparser.
                filename = check_output(['grep', 'configfile=', filename]).decode(encoding).strip()
                filename = filename.split('=', 1)[1]
            except:
                raise ConfigError('Could not fetch the index.xml path from kphotoalbumrc.')

        # Parse the XML data.

        try:
            self.dom = minidom.parse(filename)
        except:
            raise InvalidFile(sys.exc_info()[1])

        rootElement = self.dom.documentElement

        if rootElement.tagName != 'KPhotoAlbum':
            raise InvalidFile('File should be in KPhotoAlbum index.xml format.')

        if not rootElement.getAttribute('version') in ['2', '3', '4', '5', '6' ]:
            raise UnsupportedFormat('Only versions 2 to 6 are supported')

        self.isCompressed = False

        if rootElement.getAttribute('compressed') == '1':
            self.isCompressed = True

        self.categoryData = rootElement.getElementsByTagName('Categories')
        self.imageData = rootElement.getElementsByTagName('images')
        self.memberGroupsData = rootElement.getElementsByTagName('member-groups')
        self.blocklistData = rootElement.getElementsByTagName('blocklist')

    def getCategories(self):
        return CategoryIterator(self.categoryData)

    def getMediaItems(self):
        if self.isCompressed:
            return MediaItemIterator(self.imageData, self.categories)
        else:
            return MediaItemIterator(self.imageData)

    def getMemberGroups(self):
        if self.isCompressed:
            return MemberGroupIterator(self.memberGroupsData, self.categories)
        else:
            return MemberGroupIterator(self.memberGroupsData)

    def getBlockItems(self):
        return BlockItemIterator(self.blocklistData)

    categories = property(getCategories)
    mediaItems = property(getMediaItems)
    memberGroups = property(getMemberGroups)
    blockItems = property(getBlockItems)

class Error(Exception):
    """
    General error.
    """
    def __init__(self, message):
        Exception.__init__(self, message)

class ConfigError(Error):
    """
    Could not read the configuration.
    """

class InvalidFile(Error):
    """
    File is invalid.
    """

class UnsupportedFormat(InvalidFile):
    """
    File format is not supported.
    """

kpaTimeFormatStr = '%Y-%m-%dT%H:%M:%S'

def stringToDatetime(dateString):
    if not dateString:
        return None
    return datetime.fromtimestamp(mktime(strptime(dateString, kpaTimeFormatStr)))

def datetimeToString(date):
    return date.strftime(kpaTimeFormatStr)

class CategoryIterator():
    """
    Iterates categories in given DOM element.
    """
    def __init__(self, categoryData):
        self.categoryData = categoryData

    def __iter__(self):
        for categories in self.categoryData:
            for category in categories.getElementsByTagName('Category'):
                yield self.__getCategory(category)

    def __getCategory(self, categoryNode):
        a = [categoryNode.getAttribute(i)
             for i in ['name', 'icon', 'show', 'viewtype', 'thumbnailsize']]
        a[2] = bool(int(a[2])) # show
        a[3] = int(a[3]) # viewtype
        a[4] = int(a[4]) # thumbsize
        category = Category(*a)

        for valElem in categoryNode.getElementsByTagName('value'):
            name = valElem.getAttribute('value')
            idNum = int(valElem.getAttribute('id'))

            if valElem.hasAttribute('birthDate'):
                category.addItem(
                    name,
                    idNum,
                    datetime.strptime(valElem.getAttribute('birthDate'), '%Y-%m-%d').date()
                )
            else:
                category.addItem(name, idNum)

        return category

class MediaItemIterator(object):
    """
    Iterates media items in given DOM element.
    """

    def __init__(self, imageData, categories = None):
        """
        Initialize with list of images elements.

        If categories is None, will not parse compressed
        format.
        """
        self.imageData = imageData
        self.categories = categories

        if self.categories is None:
            self.categories = []

    def __iter__(self):
        for imgs in self.imageData:
            for i in imgs.getElementsByTagName('image'):
                yield self.__getMediaItem(i)

    def __getMediaItem(self, imgElem):
        a = [imgElem.getAttribute(i) for i in [
                 'file', 'md5sum', 'mediatype', 'label', 'description', 'startDate', 'endDate',
                 'width', 'height', 'angle'
            ]]

        if a[2] == '': # mediatype
            a[2] = 'image'
        elif a[2] == 'movie':
            a[2] = 'video'

        a[5] = stringToDatetime(a[5]) # startDate
        a[6] = stringToDatetime(a[6]) # endDate
        a[7] = int(a[7]) # width

        if a[7] == -1:
            a[7] = None

        a[8] = int(a[8]) # height
        if a[8] == -1:
            a[8] = None

        a[9] = int(a[9]) # angle

        img = MediaItem(*a)

        # Parse uncompressed category items
        for options in imgElem.getElementsByTagName('options'):
            for option in options.getElementsByTagName('option'):
                category = option.getAttribute('name')
                if category == 'Folder':
                    continue
                for optionValue in option.getElementsByTagName('value'):
                    item = optionValue.getAttribute('value')
                    if optionValue.hasAttribute('area'):
                        area = optionValue.getAttribute('area')
                        img.addTag(Tag(category, item, area))
                    else:
                        img.addTag(Tag(category, item))

        # Parse compressed category items
        for category in self.categories:
            if category.name == 'Folder':
                continue

            idList = imgElem.getAttribute(category.name)

            for s in idList.split(','):
                try:
                    n = int(s)
                except:
                    continue

                if n in category.items:
                    item = category.items[n]
                    img.addTag(Tag(category.name, item))
                else:
                    print('Warning: {0} has no id {1}'.format(category.name, n))

        return img

class MemberGroupIterator(object):
    """
    Iterates member groups in given DOM element.
    """

    def __init__(self, memberGroupsElements, categories = None):
        """
        Initialize with a list of member-groups elements.

        If categories is None, will not parse compressed
        format.
        """

        self.memberGroupsElements = memberGroupsElements
        self.categories = {}

        if not categories is None:
            for category in categories:
                self.categories[category.name] = category.items

    def __memberIterator(self):
        for memberGroups in self.memberGroupsElements:
            for memberGroup in memberGroups.getElementsByTagName('member'):
                yield memberGroup

    def __compressedIter(self):
        for m in self.__memberIterator():
            memberGroup = MemberGroup(*self.__getLabel(m))
            items = self.categories[memberGroup.category]
            for s in m.getAttribute('members').split(','):
                try:
                    n = int(s)
                except:
                    continue
                memberGroup.addMember(items[n])
            yield memberGroup

    def __normalIter(self):
        collected = []

        while True:
            memberIterator = self.__memberIterator()

            try:
                while True:
                    i = next(memberIterator)
                    label = self.__getLabel(i)
                    if not label in collected:
                        break

            except StopIteration:
                return

            memberGroup = MemberGroup(*label)
            memberGroup.addMember(i.getAttribute('member'))

            for i in memberIterator:
                if self.__getLabel(i) == label:
                    memberGroup.addMember(i.getAttribute('member'))
            collected += [label]
            yield memberGroup

    def __iter__(self):
        memberIterator = self.__memberIterator()

        try:
            m = next(memberIterator)
        except StopIteration:
           return iter([])

        if m.hasAttribute('members'):
            return self.__compressedIter()
        else:
            return self.__normalIter()

    def __getLabel(self, element):
        return (element.getAttribute('category'),
            element.getAttribute('group-name'))

class BlockItemIterator(object):
    """
    Iterates block items in given DOM element.
    """

    def __init__(self, blocklistElements):
        self.blocklistElements = blocklistElements

    def __iter__(self):
        for blocklist in self.blocklistElements:
            for b in blocklist.getElementsByTagName('block'):
                yield self.__getBlockItem(b)

    def __getBlockItem(self, blockNode):
        return BlockItem(blockNode.getAttribute('file'))

# vi:expandtab:tabstop=4 shiftwidth=4:
