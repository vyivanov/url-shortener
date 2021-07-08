#!/usr/bin/env bash

user=${POSTGRES_USER}
pswd=${POSTGRES_PASSWORD}
  db=${POSTGRES_DB}

if [ -z ${user} ] || [ -z ${pswd} ] || [ -z ${db} ]; then
    exit 42
fi

out=$(psql postgresql://${user}:${pswd}@postgres/${db} --command 'SELECT * FROM public.item LIMIT 1' 2>&1 /dev/null)
nok=$(echo "${out}" | grep 'does not exist')

if [[ ${nok} ]]; then
    psql postgresql://${user}:${pswd}@postgres/${db} << EOF

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

exec /root/url-shortener
