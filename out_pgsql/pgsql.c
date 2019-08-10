/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2019      Jonathan Gonzalez V. <jonathan.abdiel@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <fluent-bit/flb_info.h>
#include <fluent-bit/flb_output.h>
#include <fluent-bit/flb_pack.h>

#include "pgsql.h"

static int cb_pgsql_init(struct flb_output_instance *ins,
                          struct flb_config *config, void *data)
{

    struct flb_pgsql_config *ctx;
    const char *conn_str;

    /* default values just in case */

    if(!ins->host.name) {
        ins->host.name = flb_strdup(FLB_PGSQL_HOST);
    }

    if(!ins->host.port) {
        ins->host.port = FLB_PGSQL_PORT;
    }

    ctx = flb_calloc(1, sizeof(struct flb_pgsql_config));
    if (!ctx) {
        flb_errno();
        return -1;
    }

    ctx->db_hostname = flb_strdup(ins->host.name);
    snprintf(ctx->db_port, sizeof(ctx->db_port), "%d", ins->host.port);


    ctx->db_name = flb_output_get_property("database", ins); /* it should be dbname, may need a fix */
    if(!ctx->db_name) {
        ctx->db_name = FLB_PGSQL_DBNAME;
    }

    ctx->db_table = flb_output_get_property("table", ins);
    if(!ctx->db_table) {
        ctx->db_table = FLB_PGSQL_TABLE;
    }

    ctx->db_user = flb_output_get_property("user", ins);
    ctx->db_passwd = flb_output_get_property("passwd", ins);

    ctx->conn = PQsetdbLogin(ctx->db_hostname,
                             ctx->db_port,
                             NULL, NULL,
                             ctx->db_name,
                             ctx->db_user,
                             ctx->db_passwd);

    if(PQstatus(ctx->conn) != CONNECTION_OK) {
        flb_debug("[out_pgsql] host=%s port=%s dbname=%s %s",
                  ctx->db_hostname, ctx->db_port, ctx->db_name, PQerrorMessage(ctx->conn));
        return -1;
    }

    flb_debug("[out_pgsql] host=%s port=%s dbname=%s OK",
              ctx->db_hostname, ctx->db_port, ctx->db_name);
    flb_output_set_context(ins, ctx);

    return 0;
}

static void cb_pgsql_flush(const void *data, size_t bytes,
                            const char *tag, int tag_len,
                            struct flb_input_instance *i_ins,
                            void *out_context,
                            struct flb_config *config)
{
    struct flb_pgsql_config *ctx = out_context;
    PGresult *res;
    flb_sds_t json;
    char *query = NULL;

    if(PQstatus(ctx->conn) != CONNECTION_OK) {
        PQreset(ctx->conn);
        FLB_OUTPUT_RETURN(FLB_RETRY);
    }

    json = flb_pack_msgpack_to_json_format(data, bytes,
                                           FLB_PACK_JSON_FORMAT_JSON,
                                           FLB_PACK_JSON_DATE_DOUBLE,
                                           flb_sds_create("date"));

    query = flb_malloc(flb_sds_len(json) + 115);
    snprintf(query, flb_sds_len(json) + 115, "with json_pack as (select json_array_elements('%s') as json_pack )INSERT INTO %s select * from json_pack;", json, ctx->db_table);
    res = PQexec(ctx->conn, query);

    if(PQresultStatus(res) != PGRES_COMMAND_OK) {
        flb_debug("[out_pgsql] %s", PQerrorMessage(ctx->conn));
        PQclear(res);
        FLB_OUTPUT_RETURN(FLB_ERROR);
    }

    PQclear(res);

    FLB_OUTPUT_RETURN(FLB_OK);
}

static int cb_pgsql_exit(void *data, struct flb_config *config)
{
    struct flb_pgsql_config *ctx = data;

    PQfinish(ctx->conn);

    flb_free(ctx);

    return 0;
}

struct flb_output_plugin out_pgsql_plugin = {
    .name         = "pgsql",
    .description  = "PostgreSQL database",
    .cb_init      = cb_pgsql_init,
    .cb_flush     = cb_pgsql_flush,
    .cb_exit      = cb_pgsql_exit,
    .flags        = 0,
};
