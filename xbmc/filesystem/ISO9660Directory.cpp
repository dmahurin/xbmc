/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "ISO9660Directory.h"

#include "FileItem.h"
#include "URL.h"
#if defined(HAS_DVD_DRIVE)
#include "storage/cdioSupport.h"
#endif
#include "utils/URIUtils.h"

#include <cdio++/iso9660.hpp>

using namespace XFILE;

bool CISO9660Directory::GetDirectory(const CURL& url, CFileItemList& items)
{
  CURL url2(url);
  if (!url2.IsProtocol("iso9660"))
  {
    url2.Reset();
    url2.SetProtocol("iso9660");
    url2.SetHostName(url.Get());
  }

  std::string strRoot(url2.Get());
  std::string strSub(url2.GetFileName());

  URIUtils::AddSlashAtEnd(strRoot);
  URIUtils::AddSlashAtEnd(strSub);

  std::unique_ptr<ISO9660::IFS> iso(new ISO9660::IFS);

  std::string iso_file = url2.GetHostName();
#if defined(HAS_DVD_DRIVE)
  if (iso_file.empty())
  {
    std::shared_ptr<MEDIA_DETECT::CLibcdio> c_cdio = MEDIA_DETECT::CLibcdio::GetInstance();
    iso_file = c_cdio->GetDeviceFileName();
  }
#endif

  if (!iso->open(iso_file.c_str()))
    return false;

  std::vector<ISO9660::Stat*> isoFiles;

  if (iso->readdir(strSub.c_str(), isoFiles))
  {
    for (const auto file : isoFiles)
    {
      std::string filename(file->p_stat->filename);

      if (file->p_stat->type == 2)
      {
        if (filename != "." && filename != "..")
        {
          CFileItemPtr pItem(new CFileItem(filename));
          std::string strDir(strRoot + filename);
          URIUtils::AddSlashAtEnd(strDir);
          pItem->SetPath(strDir);
          pItem->m_bIsFolder = true;
          items.Add(pItem);
        }
      }
      else
      {
        CFileItemPtr pItem(new CFileItem(filename));
        pItem->SetPath(strRoot + filename);
        pItem->m_bIsFolder = false;
        pItem->m_dwSize = file->p_stat->size;
        items.Add(pItem);
      }
    }

    isoFiles.clear();
    return true;
  }

  return false;
}

bool CISO9660Directory::Exists(const CURL& url)
{
  CFileItemList items;
  return GetDirectory(url, items);
}
