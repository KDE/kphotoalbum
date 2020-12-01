// SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>
// SPDX-License-Identifier: CC-BY-SA-4.0
//krazy:skip
/**
  \page videothumbnails Video thumbnail related classes

  <h2>Extracting Video Length</h2>
  The class which does the actual extraction is \ref ImageManager::VideoLengthExtractor.
  It is given a filename and emits the signal
  \ref ImageManager::VideoLengthExtractor::lengthFound "lengthFound(int length)"

  The class \ref BackgroundJobs::ReadVideoLengthJob is a \ref BackgroundTaskManager::JobInterface "background job",
  which uses \ref ImageManager::VideoLengthExtractor. The job saves the length into the database.


  <h2>Extracing Thumbnails</h2>
  The class ImageManager::ExtractOneVideoFrame is the class which does the actual extraction by calling ffmpeg.
  Its main interface is the method \ref ImageManager::ExtractOneVideoFrame::extract "extract(filename,offset,receiver,slot)".
  The callback is done using the slot provided. The offset is seconds from the beginning of the video.

  \ref BackgroundJobs::ExtractOneThumbnailJob is a \ref BackgroundTaskManager::JobInterface "background job" for extracting a single
  thumbnail. It is given the video file and the offset as parameters (this time in the range 0 to 9).
  The result is stored in the .videoThumbnails directory.

  \ref BackgroundJobs::HandleVideoThumbnailRequestJob is a \ref BackgroundTaskManager::JobInterface "background job" for extracting a thumbnail
  for the thumbnail viewer. It's interface uses \ref ImageManager::ImageRequest "ImageRequest's",
  and under the hood it uses ImageManager::ExtractOneVideoFrame.


  <h2>High level classes</h2>
  \ref ThumbnailView::VideoThumbnailCycler is the class which makes the thumbnail cycle in the thumbnail viewer.
  To extract the 10 thumbnails from the videos it uses \ref ImageManager::VideoThumbnails. The main purpose of
  ImageManager::VideoThumbnails is to bridge between ThumbnailView::VideoThumbnailCycler and the backend for extracting
  the actual thumbnails. \ref ImageManager::VideoThumbnails caches the 10 found thumbnails.


  \ref BackgroundJobs::SearchForVideosWithoutLengthInfo and BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob are
  \ref BackgroundTaskManager::JobInterface "background jobs", which are started at start up of KPhotoAlbum. Their purpose
  is to check if there are video files for which the length is yet unknown, or for which there are no thumbnails for cycling.
  This could happen if the user had found the images on disk, but quit the application before it got a chance to extract
  the said informatioin.
*/
// vi:expandtab:tabstop=4 shiftwidth=4:
