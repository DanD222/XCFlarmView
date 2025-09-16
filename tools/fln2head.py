#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import csv
import requests
import re

HEADER = """\
/*
 * flarmnet_simple.h - automatisch generiert aus OGN + Flarmnet
 * nur DEVICE_ID, REGISTRATION, CN
 */
#ifndef FLARMNET_SIMPLE_H
#define FLARMNET_SIMPLE_H

typedef struct {
    unsigned int id;
    const char *reg;
    const char *cn;
} flarmnet_entry_t;

static const flarmnet_entry_t flarmnet_db[] = {
"""

FOOTER = """\
};

#endif /* FLARMNET_SIMPLE_H */
"""

URL_OGN = "http://ddb.glidernet.org/download"
URL_FLARMNET = "https://www.flarmnet.org/files/ddb.csv"

def download_file(url, local_file):
    r = requests.get(url)
    r.raise_for_status()
    with open(local_file, "wb") as f:
        f.write(r.content)
    return local_file

def clean_field(field):
    """Entfernt Quotes, Leerzeichen und prüft Inhalt"""
    return field.strip().strip("'").strip('"')

def valid_entry(reg, cn):
    """Eintrag nur gültig, wenn REG mindestens 4 alphanumerische Zeichen oder CN nicht leer"""
    if cn:
        return True
    alpha_count = len(re.findall(r'[A-Za-z0-9]', reg))
    return alpha_count >= 4

def load_csv_file(file):
    """Lädt CSV-Datei OGN oder Flarmnet robust"""
    data = {}
    with open(file, "r", encoding="latin1") as f:
        reader = csv.reader(f)
        for row in reader:
            if not row or row[0].startswith("#"):
                continue
            row = [clean_field(c) for c in row]
            if len(row) < 5:
                continue
            fid = row[1]
            reg = row[3]
            cn  = row[4]
            if valid_entry(reg, cn):
                data[fid] = {"id": fid, "reg": reg, "cn": cn}
    return data

def main():
    ogn_file = "iglide_dec.fln"
    flarm_file = "ddb.csv"
    download_file(URL_OGN, ogn_file)
    download_file(URL_FLARMNET, flarm_file)

    ogn_data = load_csv_file(ogn_file)
    flarm_data = load_csv_file(flarm_file)

    # Ergänze Flarmnet nur, wenn ID fehlt
    for fid, entry in flarm_data.items():
        if fid not in ogn_data:
            ogn_data[fid] = entry

    print(HEADER)
    sorted_keys = sorted(ogn_data.keys(), key=lambda x: int(x,16))
    for fid in sorted_keys:
        entry = ogn_data[fid]
        print(f'    {{0x{int(fid,16):06X}, "{entry["reg"]}", "{entry["cn"]}"}},')
    print(FOOTER)

if __name__ == "__main__":
    main()

