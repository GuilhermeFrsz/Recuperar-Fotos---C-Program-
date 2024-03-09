#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512

int main(int argc, char *argv[])
{
    // Verifica se o programa foi chamado com o argumento correto
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s example.raw\n", argv[0]);
        return 1;
    }

    // Abre a imagem forense para leitura
    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        fprintf(stderr, "Erro ao abrir a imagem\n");
        return 1;
    }

    // Variáveis para controle do fluxo
    uint8_t buffer[BLOCK_SIZE];
    FILE *output_file = NULL;
    char filename[8];
    int jpeg_count = 0;
    int jpeg_found = 0;

    // Loop para ler a imagem em blocos de 512 bytes
    while (fread(buffer, BLOCK_SIZE, 1, file) == 1)
    {
        // Verifica se o bloco atual é o início de um novo JPEG
        if (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && (buffer[3] & 0xf0) == 0xe0)
        {
            // Se já tiver um JPEG aberto, fecha o arquivo
            if (output_file != NULL)
            {
                fclose(output_file);
            }

            // Cria o nome do próximo arquivo JPEG
            sprintf(filename, "%03i.jpg", jpeg_count++);

            // Abre o novo arquivo JPEG para escrita
            output_file = fopen(filename, "w");
            if (output_file == NULL)
            {
                fprintf(stderr, "Erro ao criar o arquivo %s\n", filename);
                fclose(file);
                return 1;
            }

            // Escreve o bloco atual no arquivo JPEG
            fwrite(buffer, BLOCK_SIZE, 1, output_file);
            jpeg_found = 1;
        }
        else
        {
            // Se já tiver um JPEG aberto, continua escrevendo no arquivo
            if (output_file != NULL)
            {
                fwrite(buffer, BLOCK_SIZE, 1, output_file);
            }
        }
    }

    // Fecha o último arquivo JPEG, se estiver aberto
    if (output_file != NULL)
    {
        fclose(output_file);
    }

    // Fecha a imagem forense
    fclose(file);

    // Retorna 0 se pelo menos um JPEG foi encontrado, caso contrário, retorna 1
    return jpeg_found ? 0 : 1;
}
