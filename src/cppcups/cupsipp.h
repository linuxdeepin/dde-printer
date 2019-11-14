/*
 * cups - Python bindings for CUPS
 * Copyright (C) 2002, 2005, 2006, 2009  Red Hat, Inc
 * Author: Tim Waugh <twaugh@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef HAVE_CUPSIPP_H
#define HAVE_CUPSIPP_H

#include <cups/ipp.h>
#include <string>
#include <vector>

union IPPValue{
   bool b;
   int i;
   const char* s;
};

class IPPAttribute{
public:
    IPPAttribute(void);
    ~IPPAttribute(void);
    void init(ipp_tag_t group_tag,
            ipp_tag_t value_tag,
            const char *name,
            const std::vector<IPPValue>* list);
    int getGroupTag(void);
    int getValueTag(void);
    std::string getName(void);
    std::vector<IPPValue> getValues(void);

    ipp_tag_t group_tag;
    ipp_tag_t value_tag;
    std::string name;
    std::vector<IPPValue> values;
};

class IPPRequest{
public:
    IPPRequest(ipp_op_t op);
    ~IPPRequest(void);
    IPPAttribute* addSeparator(void);
    void add(IPPAttribute* attribute);
    int readIO(void* ctx, bool blocking);
    int writeIO(void* ctx, bool blocking);
    std::vector<IPPAttribute*> getAttributes(void);
    int getOperation(void);
    int getStatuscode(void);
    void setState(ipp_state_t state);
    int getState(void);
    void setStatuscode(ipp_status_t statuscode);

private:
    ipp_t *ipp;
};

#endif /* HAVE_CUPSIPP_H */
