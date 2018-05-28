/*
 * lxc_docker.h: Docker configuration import
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

#ifndef __LXC_DOCKER_H__
# define __LXC_DOCKER_H__

#include "domain_conf.h"

# define LXC_DOCKER_FORMAT "docker"

virDomainDefPtr lxcParseDockerConfig(const char *config,
                                           virDomainXMLOptionPtr xmlopt);

#endif /* __LXC_DOCKER_H__ */
