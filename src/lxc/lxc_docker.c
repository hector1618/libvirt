/*
 * lxc_docker.c: Docker Configuration import
 *
 * Copyright (C) 2018 Prafullkumar Tale
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Author: Prafullkumar Tale <talep158@gmail.com>
 */

#include <config.h>
#include <stdio.h>

#include "domain_conf.h"
#include "virlog.h"
#include "lxc_docker.h"
#include "util/virjson.h"
#include "util/virstring.h"
#include "util/viralloc.h"


#define VIR_FROM_THIS VIR_FROM_LXC

VIR_LOG_INIT("lxc.docker");


struct lxcDockerIteratorArgs {
    virDomainDefPtr def;
    size_t count;
};

static int lxcDockerCmdIteratorCallback(size_t pos ATTRIBUTE_UNUSED,
                                        virJSONValuePtr item,
                                        void *opaque)
{
    struct lxcDockerIteratorArgs *args = opaque;
    char *initarg;

    if (VIR_STRDUP(initarg, virJSONValueGetString(item)) < 0)
        goto error;

    if (!args->def->os.init) {
        if (VIR_STRDUP(args->def->os.init, initarg) < 0)
            goto error;
        else
            return 0;
    }

    if (VIR_APPEND_ELEMENT(args->def->os.initargv, args->count, initarg) < 0)
        goto error;

    return 0;

 error:
    return -1;
}

static int lxcDockerSetCmd(virDomainDefPtr def, virJSONValuePtr config)
{
    virJSONValuePtr entryPoint = NULL;
    virJSONValuePtr cmd = NULL;
    struct lxcDockerIteratorArgs callbackArg = {def, 0};

    entryPoint = virJSONValueObjectGetArray(config, "Entrypoint");
    if (entryPoint && virJSONValueIsArray(entryPoint)) {
        if (virJSONValueArrayForeachSteal(entryPoint,
                                          &lxcDockerCmdIteratorCallback,
                                          &callbackArg) < 0)

            goto error;
    }

    cmd = virJSONValueObjectGetArray(config, "Cmd");
    if (cmd && virJSONValueIsArray(cmd)) {
        if (virJSONValueArrayForeachSteal(cmd,
                                          &lxcDockerCmdIteratorCallback,
                                          &callbackArg) < 0)
            goto error;
    }

    /* Append NULL element */
    if (callbackArg.count > 0 &&
        VIR_EXPAND_N(callbackArg.def->os.initargv,
                     callbackArg.count, 1) < 0)
        goto error;

    return 0;

 error:
    return -1;
}

virDomainDefPtr lxcParseDockerConfig(const char *config,
                                     virDomainXMLOptionPtr xmlopt)
{
    virDomainDefPtr vmdef = NULL;
    virJSONValuePtr parentJsonObj = NULL;
    virJSONValuePtr jsonObj = NULL;

    if (!(parentJsonObj = virJSONValueFromString(config))) {
        virReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("failed to parse JSON config"));
        return NULL;
    }

    if (!(jsonObj = virJSONValueObjectGetObject(parentJsonObj, "config")))
        return NULL;

    if (!(vmdef = virDomainDefNew()))
        goto error;

    if (virUUIDGenerate(vmdef->uuid) < 0) {
        virReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("failed to generate uuid"));
        goto error;
    }

    vmdef->id = -1;
    virDomainDefSetMemoryTotal(vmdef, 64 * 1024);

    vmdef->onReboot = VIR_DOMAIN_LIFECYCLE_ACTION_RESTART;
    vmdef->onCrash = VIR_DOMAIN_LIFECYCLE_ACTION_DESTROY;
    vmdef->onPoweroff = VIR_DOMAIN_LIFECYCLE_ACTION_DESTROY;
    vmdef->virtType = VIR_DOMAIN_VIRT_LXC;

    /* Value not handled by the LXC driver, setting to
     * minimum required to make XML parsing pass
     */
    if (virDomainDefSetVcpusMax(vmdef, 1, xmlopt) < 0)
        goto error;

    if (virDomainDefSetVcpus(vmdef, 1) < 0)
        goto error;

    vmdef->os.type = VIR_DOMAIN_OSTYPE_EXE;
    if (lxcDockerSetCmd(vmdef, jsonObj) < 0) {
        virReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("failed to parse Cmd"));
        goto error;
    }

    goto cleanup;

 error:
    virDomainDefFree(vmdef);
    vmdef = NULL;

 cleanup:
    return vmdef;
}
