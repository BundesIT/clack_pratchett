/** @file
 *  CLACK Plugin to honor Terry Pratchett 
 *  http://www.nerdcore.de/2015/03/14/gnu-terry-pratchett-http-header/
 * 
 *  @section license License
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */




#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <ts/ts.h>

static TSMBuffer hdr_bufp;
static TSMLoc hdr_loc;


static void
clack_pratchett(TSHttpTxn txnp, TSCont contp)
{
    TSMBuffer resp_bufp;
    TSMLoc resp_loc;
    TSMLoc field_loc;
    TSMLoc next_field_loc;
    TSMLoc new_field_loc;
    int retval;

    if (TSHttpTxnServerRespGet(txnp, &resp_bufp, &resp_loc) != TS_SUCCESS) {
        TSError("[add_header] Error while retrieving server response header\n");
        goto done;
    }

    field_loc = TSMimeHdrFieldGet(hdr_bufp, hdr_loc, 0);
    if (field_loc == TS_NULL_MLOC) {
        TSError("[add_header] Error while getting field");
        goto error;
    }

    if (TSMimeHdrFieldCreate(resp_bufp, resp_loc, &new_field_loc) != TS_SUCCESS) {
        TSError("[add_header] Error while creating new field");
        TSHandleMLocRelease(hdr_bufp, hdr_loc, field_loc);
        goto error;
    }

    retval = TSMimeHdrFieldCopy(resp_bufp, resp_loc, new_field_loc, hdr_bufp, hdr_loc, field_loc);
    if (retval == TS_ERROR) {
        TSError("[add_header] Error while copying new field");
        TSHandleMLocRelease(hdr_bufp, hdr_loc, field_loc);
        goto error;
    }

    retval = TSMimeHdrFieldAppend(resp_bufp, resp_loc, new_field_loc);
    if (retval != TS_SUCCESS) {
        TSError("[add_header] Error while appending new field");
        TSHandleMLocRelease(hdr_bufp, hdr_loc, field_loc);
        goto error;
    }

    TSHandleMLocRelease(hdr_bufp, hdr_loc, field_loc);

error:
    TSHandleMLocRelease(resp_bufp, TS_NULL_MLOC, resp_loc);

done:
    TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);
}

static int
clack_partchett_plugin(TSCont contp, TSEvent event, void *edata)
{
    TSHttpTxn txnp = (TSHttpTxn) edata;

    switch (event) {
    case TS_HTTP_READ_RESPONSE_HDR_HOOK:
        clack_pratchett(txnp, contp);
        return 0;
    default:
        break;
    }
    return 0;
}

void TSPluginInit(int argc, const char *argv[]) {
    TSMLoc field_loc;
    const char *p;
    int i, retval;
    TSPluginRegistrationInfo info;

    info.plugin_name = "clack-pratchett";
    info.vendor_name = "Piratenpartei Deutschland";
    info.support_email = "bundesit@lists.piratenpartei.de";

    if (TSPluginRegister(TS_SDK_VERSION_3_0, &info) != TS_SUCCESS) {
        TSError("[PluginInit] Plugin registration failed.\n");
        goto error;
    }

    hdr_bufp = TSMBufferCreate();
    if (TSMimeHdrCreate(hdr_bufp, &hdr_loc) != TS_SUCCESS) {
        TSError("[PluginInit] Can not create mime header");
        goto error;
    }
    if (TSMimeHdrFieldCreate(hdr_bufp, hdr_loc, &field_loc) != TS_SUCCESS) {
        TSError("[PluginInit] Error while creating field");
        goto error;
    }
    retval = TSMimeHdrFieldAppend(hdr_bufp, hdr_loc, field_loc);
    if (retval != TS_SUCCESS) {
        TSError("[PluginInit] Error while adding field");
        goto error;
    }
    retval = TSMimeHdrFieldNameSet(hdr_bufp, hdr_loc, field_loc, "X-Clacks-Overhead", 17);
    if (retval == TS_ERROR) {
        TSError("[PluginInit] Error while naming field");
        goto error;
    }
    retval = TSMimeHdrFieldValueStringInsert(hdr_bufp, hdr_loc, field_loc, -1, "GNU Terry Pratchett", 19);
    if (retval == TS_ERROR) {
        TSError("[PluginInit] Error while inserting field value");
        goto error;
    }

    TSHttpHookAdd(TS_HTTP_READ_RESPONSE_HDR_HOOK, TSContCreate(clack_partchett_plugin, TSMutexCreate()));
    goto done;

error:
    TSError("[PluginInit] Plugin not initialized");

done:
    return;
}
