/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Wei xie <xiewei@deepin.com>
 *
 * Maintainer: Wei xie  <xiewei@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CUPSATTRNAMES_H
#define CUPSATTRNAMES_H

#define CUPS_DEV_CLASS      "device-class"
#define CUPS_DEV_URI        "device-uri"
#define CUPS_DEV_ID         "device-id"
#define CUPS_DEV_INFO       "device-info"
#define CUPS_DEV_LOCATION   "device-location"
#define CUPS_DEV_MAKE_MODE  "device-make-and-model"

#define CUPS_OP_COPIES      "copies"
#define CUPS_OP_FINISH      "finishings"
#define CUPS_OP_TIME        "marker-change-time"
#define CUPS_OP_COLORS      "marker-colors"
#define CUPS_OP_LEVELS      "marker-levels"
#define CUPS_OP_MNAME       "marker-names"
#define CUPS_OP_MTYPES      "marker-types"
#define CUPS_OP_MEDIA       "media"
#define CUPS_OP_NUMBER      "number-up"
#define CUPS_OP_BIN         "output-bin"
#define CUPS_OP_COLOR_MODE  "print-color-mode"
#define CUPS_OP_QUALITY     "print-quality"
#define CUPS_OP_COMMANDS    "printer-commands"
#define CUPS_OP_INFO        "printer-info"
#define CUPS_OP_NAME        "printer-name"
#define CUPS_OP_ISACCEPT    "printer-is-accepting-jobs"
#define CUPS_OP_ISSHAED     "printer-is-shared"
#define CUPS_OP_LOCATION    "printer-location"
#define CUPS_OP_MAKE_MODEL  "printer-make-and-model"
#define CUPS_OP_STATE       "printer-state"
#define CUPS_OP_STATE_TIME  "printer-state-change-time"
#define CUPS_OP_STATIE_RES  "printer-state-reasons"
#define CUPS_OP_TYPE        "printer-type"
#define CUPS_OP_URI_SUP     "printer-uri-supported"
#define CUPS_OP_POLICY      "printer-op-policy"
#define CUPS_OP_UP_TIME     "printer-up-time"

#define CUPS_PPD_NAME       "ppd-name"
#define CUPS_PPD_LANG       "ppd-natural-language"
#define CUPS_PPD_MAKE       "ppd-make"
#define CUPS_PPD_MODEL      "ppd-model"
#define CUPS_PPD_MAKE_MODEL "ppd-make-and-model"
#define CUPS_PPD_ID         "ppd-device-id"
#define CUPS_PPD_PRODUCT    "ppd-product"
#define CUPS_PPD_VER        "ppd-psversion"
#define CUPS_PPD_TYPE       "ppd-type"
#define CUPS_PPD_NUMBER     "ppd-model-number"

#define JOB_ATTR_CANCEL     "job-cancel-after"
#define JOB_ATTR_HOLD       "job-hold-until"
#define JOB_ATTR_SHEETS     "job-sheets"
#define JOB_ATTR_ID         "job-id"
#define JOB_ATTR_SIZE       "job-k-octets"
#define JOB_ATTR_NAME       "job-name"
#define JOB_ATTR_USER       "job-originating-user-name"
#define JOB_ATTR_STATE_MEG  "job-printer-state-message"
#define JOB_ATTR_STATE_RES  "job-printer-state-reasons"
#define JOB_ATTR_URI        "job-printer-uri"
#define JOB_ATTR_STATE_STR  "job-state-reasons"
#define JOB_ATTR_TIME_ADD   "time-at-creation"
#define JOB_ATTR_TIME_END   "time-at-completed"
#define JOB_ATTR_STATE      "job-state"
#define JOB_ATTR_PRIORITY   "job-priority"
#define JOB_ATTR_IMPRES     "job-impressions-completed"
#define JOB_ATTR_DOC_NUM    "number-of-documents"

#define CUPS_NOTIY_CHARSET  "notify-charset"
#define CUPS_NOTIY_JOBID    "notify-job-id"
#define CUPS_NOTIY_LAN      "notify-natural-language"
#define CUPS_NOTIY_UIR      "notify-printer-uri"
#define CUPS_NOTIY_SEQ_NUM  "notify-sequence-number"
#define CUPS_NOTIY_EVENT    "notify-subscribed-event"
#define CUPS_NOTIY_SUBID    "notify-subscription-id"
#define CUPS_NOTIY_TEXT     "notify-text"

#endif// CUPSATTRNAMES_H
