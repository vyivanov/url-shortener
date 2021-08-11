#!/usr/bin/env bash

readonly USER=${POSTGRES_USER}
readonly PSWD=${POSTGRES_PASSWORD}
readonly   DB=${POSTGRES_DB}

if [ -z "${USER}" ] || [ -z "${PSWD}" ] || [ -z "${DB}" ]; then
    echo 'bad database config' >&2
    exit 42
fi

readonly EP="postgresql://${USER}:${PSWD}@storage:5432/${DB}"

out=$(wait-for-it storage:5432 --timeout=60 --quiet -- \
    psql "${EP}" --command 'SELECT * FROM public.item LIMIT 1' 2>&1 /dev/null)

nok=$(echo "${out}" | grep 'does not exist')

if [[ ${nok} ]]; then
    psql "${EP}" << EOF

CREATE TABLE public.item
(
    idx serial NOT NULL,
    url text NOT NULL,
    tms timestamp NOT NULL DEFAULT timezone('UTC'::text, now()),
    ipc inet NOT NULL,
    cnt int NOT NULL DEFAULT 0,
    CONSTRAINT item_pk PRIMARY KEY (idx)
);

COMMENT ON COLUMN public.item.idx IS 'key to be provided to client';
COMMENT ON COLUMN public.item.url IS 'link to be redirected to';
COMMENT ON COLUMN public.item.tms IS 'utc timestamp the record was created at';
COMMENT ON COLUMN public.item.ipc IS 'client ip the record was created from';
COMMENT ON COLUMN public.item.cnt IS 'redirection counter';

EOF
fi

exec /var/www/url-shortener
