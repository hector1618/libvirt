/*
 * lxc_docker.c: Docker Configuration import
 *
 * Copyright (C) 2018 Prafullkumar T
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
 * Author: Prafullkumar T <talep158@gmail.com>
 */

#include <config.h>
#include <stdio.h>

#include "domain_conf.h"
#include "virlog.h"
#include "lxc_docker.h"
#include "util/virstring.h"
#include "util/viralloc.h"
#include "util/virjson.h"


#define VIR_FROM_THIS VIR_FROM_LXC

VIR_LOG_INIT("lxc.docker");

struct lxcDockerEnv {
    virDomainDefPtr def;
    size_t envCount;
};

static int lxcDockerSetWorkingDir(virDomainDefPtr def, virJSONValuePtr config)
{
    const char *workingDir = NULL;

    if(!(workingDir = virJSONValueObjectGetString(config, "WorkingDir")))
        goto error;

    if(strcmp(workingDir, "") == 0 || VIR_STRDUP(def->os.initdir, workingDir) < 0)
        goto cleanup;

    return 1;

error:
    return -1;

cleanup:
    return 0;
}

static int lxcDockerEnvIteratorCallback(size_t pos ATTRIBUTE_UNUSED,
                                        virJSONValuePtr item,
                                        void *opaque) {

    char *tmp;
    char **parts;
    struct lxcDockerEnv *args = opaque;

    if (VIR_STRDUP(tmp, virJSONValueGetString(item)) < 0 || !tmp)
        return 0;

    if (!(parts = virStringSplit(tmp, "=", 0)))
        goto cleanup;

    if (!parts[0] || !parts[1])
        goto cleanup;

    if (VIR_EXPAND_N(args->def->os.initenv, args->envCount, 1) < 0)
        goto  error;
    if (VIR_ALLOC(args->def->os.initenv[args->envCount-1]) < 0)
        goto error;

    if (VIR_STRDUP(args->def->os.initenv[args->envCount-1]->name, parts[0]) < 0)
        goto error;
    if (VIR_STRDUP(args->def->os.initenv[args->envCount-1]->value, parts[1]) < 0)
        goto error;

    goto cleanup;

error:
    VIR_FREE(tmp);
    return -1;

cleanup:
    virStringListFree(parts);
    VIR_FREE(tmp);
    return 1;
}

static int lxcDockerSetEnv(virDomainDefPtr def,
                           virJSONValuePtr config) {

    virJSONValuePtr env = NULL;
    struct lxcDockerEnv  callbackArg = {def, 0};


    if(!(env = virJSONValueObjectGetArray(config, "Env")))
            goto cleanup;

    if(virJSONValueArrayForeachSteal(env,
                                     &lxcDockerEnvIteratorCallback,
                                     &callbackArg) < 0)
        goto error;

    if(VIR_EXPAND_N(def->os.initenv, callbackArg.envCount, 1) < 0)
        goto error;

    return 1;

cleanup:
    return 0;
error:
    return -1;
}

static int lxcDockerCmdIteratorCallback(size_t pos ATTRIBUTE_UNUSED,
                                        virJSONValuePtr item,
                                        void *opaque) {

    struct lxcDockerEnv *args = opaque;
    const char *initarg = virJSONValueGetString(item);

    if(!args->def->os.init) {
        if (VIR_STRDUP(args->def->os.init, initarg) < 0)
            goto error;
        else {
            return 1;
        }
    }

    if (VIR_EXPAND_N(args->def->os.initargv, args->envCount, 1) < 0)
        goto error;
    if (VIR_STRDUP(args->def->os.initargv[args->envCount-1], initarg) < 0)
        goto error;

    return 1;

error:
    return -1;
}

static int lxcDockerSetCmd(virDomainDefPtr def, virJSONValuePtr config) {

    virJSONValuePtr entryPoint = NULL;
    virJSONValuePtr cmd = NULL;
    struct lxcDockerEnv callbackArg = {def, 0};

    if ((entryPoint = virJSONValueObjectGetArray(config, "Entrypoint")) &&
            virJSONValueIsArray(entryPoint)) {
        if(virJSONValueArrayForeachSteal(entryPoint,
                                         &lxcDockerCmdIteratorCallback,
                                         &callbackArg) < 0)

            goto error;
    }

    if ((cmd = virJSONValueObjectGetArray(config, "Cmd")) &&
        virJSONValueIsArray(cmd)) {
        if(virJSONValueArrayForeachSteal(cmd,
                                         &lxcDockerCmdIteratorCallback,
                                         &callbackArg) < 0)
            goto error;
    }

    if (callbackArg.envCount > 0 &&
        VIR_EXPAND_N(callbackArg.def->os.initargv,
                     callbackArg.envCount, 1) < 0)
        goto error;

    return 1;

error:
    return -1;
}

static int lxcDockerSetUser(virDomainDefPtr def, virJSONValuePtr config)
{
    const char *user= NULL;

    if(!(user = virJSONValueObjectGetString(config, "User")))
        goto error;

    if(strcmp(user, "") == 0 || VIR_STRDUP(def->os.inituser, user) < 0)
        goto cleanup;

    return 1;

error:
    return -1;

cleanup:
    return 0;
}

static const char *lxcDockerSetArch(virJSONValuePtr config)
{
    const char *archStr = NULL;


    if(!(archStr = virJSONValueObjectGetString(config, "architecture")))
        return NULL;

    if(strcmp(archStr, "") == 0)
        return NULL;

    return archStr;
}

virDomainDefPtr lxcParseDockerConfig(const char *config ATTRIBUTE_UNUSED,
                                           virDomainXMLOptionPtr xmlopt ATTRIBUTE_UNUSED) {

    virDomainDefPtr vmdef = NULL;
    virJSONValuePtr parentJsonObj = NULL;
    virJSONValuePtr jsonObj = NULL;
    const char *archStr = NULL;

    if (!(parentJsonObj = virJSONValueFromString(config)))
        return NULL;

    // #TODO
    // Figure out appropriate config JSON object
    // to read the config parameters
    if(!(jsonObj = virJSONValueObjectGetObject(parentJsonObj, "config")))
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
    * minimum required to make XML parsing pass */
    if (virDomainDefSetVcpusMax(vmdef, 1, xmlopt) < 0)
        goto error;

    if (virDomainDefSetVcpus(vmdef, 1) < 0)
        goto error;

    vmdef->nfss = 0;
    vmdef->os.type = VIR_DOMAIN_OSTYPE_EXE;


    archStr = lxcDockerSetArch(parentJsonObj);
    if(archStr != NULL) {
        virArch arch = virArchFromString(archStr);
        if (arch == VIR_ARCH_NONE && STREQ(archStr, "x86"))
            arch = VIR_ARCH_I686;
        else if (arch == VIR_ARCH_NONE && STREQ(archStr, "amd64"))
            arch = VIR_ARCH_X86_64;
        vmdef->os.arch = arch;
    }

    if(lxcDockerSetWorkingDir(vmdef, jsonObj) < 0) {
        virReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("failed to parse WorkingDir"));
    }

    if(lxcDockerSetEnv(vmdef, jsonObj) < 0) {
        virReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("failed to parse Env"));
    }

    if(lxcDockerSetCmd(vmdef, jsonObj) < 0) {
        virReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("failed to parse Cmd"));
    }

    if(lxcDockerSetUser(vmdef, jsonObj) < 0) {
        virReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                       _("failed to parse User"));
    }

    goto cleanup;

error:
    virDomainDefFree(vmdef);
    vmdef = NULL;

cleanup:
    return vmdef;
}
