#include "../diag_symmetry/diag_symm.hpp"
#include "../combination_encoding/combination_encoding.hpp"
#include "../config.hpp"
#include "write_to_files.hpp"

#include <stdio.h>
void write_to_files()
{
    char filename[512];
    FILE *fp;
    sprintf(filename, "%s/encode_tables/without_cut/diag_symm_tables.hpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#ifndef __enc_diag_symm_tables__\n");
    fprintf(fp, "#define __enc_diag_symm_tables__ 1\n");
    fprintf(fp, "extern const int compressed_comb_count[7];\n");
    fprintf(fp, "extern const int convert_to_compressed_comb_index[7][100947];\n");
    fprintf(fp, "#endif\n");
    fclose(fp);

    sprintf(filename, "%s/encode_tables/without_cut/diag_symm_tables.cpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#include \"diag_symm_tables.hpp\"\n");
    fprintf(fp, "const int compressed_comb_count[7] = {");
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        fprintf(fp, "%d,", compressed_comb_count[big_num]);
    }
    fprintf(fp, "};\n");
    fprintf(fp, "const int convert_to_compressed_comb_index[7][100947] = {\n");
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        fprintf(fp, "\t{");
        for (int comb_id = 0; comb_id < C[POOL_SIZE][big_num]; comb_id++)
        {
            fprintf(fp, "%d,", convert_to_compressed_comb_index[big_num][comb_id]);
        }
        fprintf(fp, "},\n");
    }
    fprintf(fp, "};\n");
    fclose(fp);

    sprintf(filename, "%s/decode_tables/without_cut/diag_symm_tables.hpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#ifndef __dec_diag_symm_tables__\n");
    fprintf(fp, "#define __dec_diag_symm_tables__ 1\n");
    fprintf(fp, "extern const int decompress_comb_index[7][%d];\n", compressed_comb_count[6]);
    fprintf(fp, "#endif\n");
    fclose(fp);

    sprintf(filename, "%s/decode_tables/without_cut/diag_symm_tables.cpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#include \"diag_symm_tables.hpp\"\n");
    fprintf(fp, "const int decompress_comb_index[7][%d] = {\n", compressed_comb_count[6]);
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        fprintf(fp, "\t{");
        for (int comb_id = 0; comb_id < compressed_comb_count[big_num]; comb_id++)
        {
            fprintf(fp, "%d,", decompress_comb_index[big_num][comb_id]);
        }
        fprintf(fp, "},\n");
    }
    fprintf(fp, "};\n");
    fclose(fp);

    // write diag symmetry with cut
    sprintf(filename, "%s/encode_tables/with_cut/diag_symm_tables.hpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#ifndef __enc_diag_symm_tables_cut__\n");
    fprintf(fp, "#define __enc_diag_symm_tables_cut__ 1\n");
    fprintf(fp, "extern const int compressed_cut_comb_count[7];\n");
    fprintf(fp, "extern const int convert_to_compressed_cut_comb_index[7][12376];\n");
    fprintf(fp, "#endif\n");
    fclose(fp);

    sprintf(filename, "%s/encode_tables/with_cut/diag_symm_tables.cpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#include \"diag_symm_tables.hpp\"\n");
    fprintf(fp, "const int compressed_cut_comb_count[7] = {");
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        fprintf(fp, "%d,", compressed_cut_comb_count[big_num]);
    }
    fprintf(fp, "};\n");
    fprintf(fp, "const int convert_to_compressed_cut_comb_index[7][12376] = {\n");
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        fprintf(fp, "\t{");
        for (int comb_id = 0; comb_id < C[POOL_SIZE_CUT][big_num]; comb_id++)
        {
            fprintf(fp, "%d,", convert_to_compressed_cut_comb_index[big_num][comb_id]);
        }
        fprintf(fp, "},\n");
    }
    fprintf(fp, "};\n");
    fclose(fp);

    sprintf(filename, "%s/decode_tables/with_cut/diag_symm_tables.hpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#ifndef __dec_diag_symm_tables_cut__\n");
    fprintf(fp, "#define __dec_diag_symm_tables_cut__ 1\n");
    fprintf(fp, "extern const int decompress_cut_comb_index[7][%d];\n", compressed_cut_comb_count[6]);
    fprintf(fp, "#endif\n");
    fclose(fp);

    sprintf(filename, "%s/decode_tables/with_cut/diag_symm_tables.cpp", TABLE_PREFIX);
    fp = fopen(filename, "w");
    fprintf(fp, "#include \"diag_symm_tables.hpp\"\n");
    fprintf(fp, "const int decompress_cut_comb_index[7][%d] = {\n", compressed_cut_comb_count[6]);
    for (int big_num = 0; big_num <= 6; big_num++)
    {
        fprintf(fp, "\t{");
        for (int comb_id = 0; comb_id < compressed_cut_comb_count[big_num]; comb_id++)
        {
            fprintf(fp, "%d,", decompress_cut_comb_index[big_num][comb_id]);
        }
        fprintf(fp, "},\n");
    }
    fprintf(fp, "};\n");
    fclose(fp);
}