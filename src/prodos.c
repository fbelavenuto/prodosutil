/*  ProDos - Utilitário para manipulação de volumes prodos
 *  by Fabio Belavenuto - Copyright 2004
 *
 *  Este arquivo ~´e distribuido pela Licença Pública Geral GNU.
 *  Veja o arquivo "Licenca.txt" distribuido com este software.
 *
 *  ESTE SOFTWARE NÃO OFERECE NENHUMA GARANTIA
 */

#include <stdlib.h>
#include <stdio.h>
//#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "version.h"
#include "prodos.h"
#include "prodosBoot.h"

// Variaveis:
FILE              *arqEntrada=NULL, *arqSaida=NULL;
TEntradaDirVol    volume;
TEntradaDirVol    diretorio;
TEntradaArquivo   arquivos[PRODOS_MAXARQS];
int               numEntradas;
byte              bitMap[8192];
byte              eImagemDSK = 0;

// Funções

/*****************************************************************************/
void sair(int erro, char *texto) {
	if (texto) {
		fprintf(stderr, "%s\n", texto);
	}
	if (arqEntrada) {
		fclose(arqEntrada);
	}
	if (arqSaida) {
		fclose(arqSaida);
	}
	exit(erro);
}

/*****************************************************************************/
int posicionar(int trilha, int setor) {
	if (trilha > 34 || setor > 15)
		sair(ERR_ERROLEITURA, "Trilha e/ou setor fora do intervalo.\n");
    return fseek(arqEntrada, (trilha * 16uL + setor)*256, SEEK_SET);
}

/*****************************************************************************/
void lerBloco(int bloco, char buffer[512]) {
	int trilha, setor1, setor2;

	if (eImagemDSK) {
		trilha = (bloco / 8);
		setor1 = setoresProDos[(bloco % 8)*2];
		setor2 = setoresProDos[(bloco % 8)*2+1];
		posicionar(trilha, setor1);
		fread(buffer, 256, 1, arqEntrada);
		posicionar(trilha, setor2);
		fread(buffer+256, 256, 1, arqEntrada);
	} else {
		fseek(arqEntrada, bloco * 512, SEEK_SET);
		fread(buffer, 512, 1, arqEntrada);
	}
}

/*****************************************************************************/
void escreverBloco(int bloco, char buffer[512]) {
	int trilha, setor1, setor2;

	if (eImagemDSK) {
		trilha = (bloco / 8);
		setor1 = setoresProDos[(bloco % 8)*2];
		setor2 = setoresProDos[(bloco % 8)*2+1];
		posicionar(trilha, setor1);
		fwrite(buffer, 256, 1, arqEntrada);
		posicionar(trilha, setor2);
		fwrite(buffer+256, 256, 1, arqEntrada);
	} else {
		fseek(arqEntrada, bloco * 512, SEEK_SET);
		fwrite(buffer, 512, 1, arqEntrada);
	}
}

/*****************************************************************************/
void obtemDataProDos(word *wData, word *wHora) {
	int Dia, Mes, Ano, Hora, Minuto;
	struct tm *data;
	time_t tempo;

	tempo = time(NULL);
	data = localtime(&tempo);

	Dia = data->tm_mday;
	Mes = data->tm_mon+1;
	Ano = data->tm_year % 100; 
	Hora = data->tm_hour;
	Minuto = data->tm_min;

	*wData = ((Ano & 0x7F) << 9) | ((Mes & 0x0F) << 5) | (Dia & 0x1F);
	*wHora = ((Hora & 0x1F) << 8) | (Minuto & 0x3F);
}

/*****************************************************************************/
void converteDataProDos(char* str, word wData, word wHora) {
	int  Ano, Mes, Dia, Hora, Minuto;
	const char Meses[12][4] = {
		"JAN\0", "FEV\0", "MAR\0", "ABR\0", "MAI\0", "JUN\0",
		"JUL\0", "AGO\0", "SET\0", "OUT\0", "NOV\0", "DEZ\0"
	};

	Ano = (wData & 0xFE00) >> 9;
	Mes = (wData & 0x01E0) >> 5;
	Dia = (wData & 0x001F);
	Hora = (wHora & 0x1F00) >> 8;
	Minuto = (wHora & 0x003F);
	if ((!wData && !wHora) || (Mes < 1 || Mes > 12)) {
		strcpy(str, "   <sem data>  ");
		return;
	}

	sprintf(str,"%2d-%s-%.2d %2d:%.2d", Dia, Meses[Mes-1], Ano, Hora, Minuto);
}

/*****************************************************************************/
void verificaNomeApple(char *nomeApple) {
	int i, ok = 0;

	if (strlen(nomeApple) > 15) {
		nomeApple[15] = '\0';
	}

	for (i=0; i < strlen(nomeApple); i++)
	  nomeApple[i] = toupper(nomeApple[i]);

	if (nomeApple[0] >= 'A' && nomeApple[0] <= 'Z') {
		ok = 1;
	}

	for (i=0; i<strlen(nomeApple); i++) {
		if (!(  (nomeApple[i] >= 'A' && nomeApple[i] <= 'Z') ||
				(nomeApple[i] >= '0' && nomeApple[i] <= '9') ||
				(nomeApple[i] == '.')
		)) {
			ok = 0;
			break;
		}

	}
	if (!ok) {
		sair(ERR_ERROLINHACOMANDO, "Nome Apple2 invalido");
	}
}

/*****************************************************************************/
void verificaVolume(void) {
	char buffer[512];
	byte tipoEnt;
	int tamArq, i, tB;
	int totalBlocos;

	fseek(arqEntrada, 0, SEEK_END);
	tamArq = ftell(arqEntrada);
	fseek(arqEntrada, 0, SEEK_SET);

	if (tamArq == 143360) {
		eImagemDSK = 1;
	}

	lerBloco(2, buffer);
	memcpy(&volume, buffer+4, sizeof(TEntradaDirVol));
	totalBlocos = (int)volume.volTotalBlocosDoVolume;
	tipoEnt = (volume.tipoArq_LenNome & 0xF0) >> 4;
	if ((!totalBlocos) || (tipoEnt != PRODOS_TA_CDIR && tipoEnt != PRODOS_TA_CVOL))
		sair(ERR_NAORECONHECIDO, "Imagem de disco nao é um disco ProDos ou está corrompido");
	tB = (totalBlocos-1) / 4096;
	for (i = volume.volBlocoBitMap; i <= volume.volBlocoBitMap + tB; i++) {
		lerBloco(i, (char *)bitMap+(i-volume.volBlocoBitMap)*512);
	}
}

/*****************************************************************************/
int blocoEstaLivre(int bloco) {
	return bitIsSet(7 - (bloco % 8), bitMap[bloco / 8]);
}

/*****************************************************************************/
void liberaBloco(int bloco) {
	setBit(7-(bloco % 8), bitMap[bloco / 8]);
}

/*****************************************************************************/
void ocupaBloco(int bloco) {
	clearBit(7-(bloco % 8), bitMap[bloco / 8]);
}

/*****************************************************************************/
int retornaBlocosLivres() {
	int result = 0, i;
	int totalBlocos = (int)volume.volTotalBlocosDoVolume;

	for (i = 0; i < totalBlocos; i++) {
		if (blocoEstaLivre(i)) ++result;
	}

	return result;
}

/*****************************************************************************/
int achaBlocoLivre() {
	int i;
	int totalBlocos = (int)volume.volTotalBlocosDoVolume;

	for (i = 0; i < totalBlocos; i++) {
		if (blocoEstaLivre(i)) return i;
	}

	return 0;
}

/*****************************************************************************/
void salvaDir(int bloco) {
	char  buffer[512];
	int   indice = 0;
	int   proximoBloco = bloco;
	int   blocoAtual = 0;
	int   byteAtual = 4;

	lerBloco(proximoBloco, buffer);
	blocoAtual = proximoBloco;
	memcpy(&proximoBloco, buffer+2, sizeof(word));
	memcpy(buffer+byteAtual, &diretorio, sizeof(TEntradaDirVol));

	byteAtual += sizeof(TEntradaDirVol);
	while(indice <= numEntradas) {
		memcpy(buffer+byteAtual, &arquivos[indice++], sizeof(TEntradaArquivo));
		byteAtual += sizeof(TEntradaArquivo);
		if (byteAtual >= 511) {
			escreverBloco(blocoAtual, buffer);
			blocoAtual = 0;
			if (!proximoBloco) {
				break;
			}
			lerBloco(proximoBloco, buffer);
			blocoAtual = proximoBloco;
			memcpy(&proximoBloco, buffer+2, sizeof(word));
			byteAtual = 4;
		}
	}
	if (blocoAtual) {
		escreverBloco(blocoAtual, buffer);
	}
}

/*****************************************************************************/
void removeArquivo(int blocoDir, int numEntrada) {
	char indices[1024];
	int bloco, tipoEnt, t, x;

	tipoEnt = (arquivos[numEntrada].tipoArq_LenNome & 0xF0) >> 4;

	if (tipoEnt == PRODOS_TA_DEL || tipoEnt > PRODOS_TA_TREE)
		sair(ERR_ARQUIVOEDIRETORIO, "Impossivel deletar arquivo");

	arquivos[numEntrada].tipoArq_LenNome = PRODOS_TA_DEL << 4;
	diretorio.entradasAtivas--;

	salvaDir(blocoDir);

	bloco = arquivos[numEntrada].blocoArquivo;

	switch(tipoEnt) {
		case PRODOS_TA_SEED:		// arquivo "seedling" (usa somente um bloco)
			liberaBloco(bloco);
		break;

		case PRODOS_TA_SAPL:		// arquivo "sapling" (usa de 2 até 256 blocos)
			liberaBloco(bloco);
			lerBloco(bloco, indices);
			for (t = 0; t < 256; t++) {
				bloco = (indices[t + 256] << 8) + indices[t];
				if (bloco != 0) {
					liberaBloco(bloco);
				} // if bloco != 0
			} // for t
		break;

		case PRODOS_TA_TREE:		// arquivo "tree" (usa de 257 blocos até 32768 blocos)
			liberaBloco(bloco);
			lerBloco(bloco, indices);
			for (t = 0; t < 256; t++) {
				bloco = (indices[t + 256] << 8) + indices[t];
				if (bloco != 0) {
					liberaBloco(bloco);
					lerBloco(bloco, indices+512);
					for (x = 0; x < 256; x++) {
						bloco = (indices[512 + x + 256] << 8) + indices[512 + x];
						if (bloco != 0) {
							liberaBloco(bloco);
						} // if bloco != 0
					} // for x
				} // if bloco != 0
			} // for t
		break;
	}

	escreverBloco(volume.volBlocoBitMap, (char *)bitMap);
}

/*****************************************************************************/
void carregaDir(int bloco) {
	char  buffer[512];
	int   indice = 0, c = 0, tipoEnt;
	int   proximoBloco = bloco;
	int   byteAtual = 4;

	memset(arquivos, 0, sizeof(TEntradaArquivo) * PRODOS_MAXARQS);
	lerBloco(proximoBloco, buffer);
	memcpy(&proximoBloco, buffer+2, sizeof(word));
	memcpy(&diretorio, buffer+byteAtual, sizeof(TEntradaDirVol));

	if (!diretorio.entradasAtivas) {
		numEntradas = 0;
		return;
	}

	byteAtual += sizeof(TEntradaDirVol);
	tipoEnt = (volume.tipoArq_LenNome & 0xF0) >> 4;
	if (tipoEnt != PRODOS_TA_CDIR && tipoEnt != PRODOS_TA_CVOL) {
		numEntradas = 0;
		return;
	}
	while(c < diretorio.entradasAtivas) {
		memcpy(&arquivos[indice], buffer+byteAtual, sizeof(TEntradaArquivo));
		byteAtual += sizeof(TEntradaArquivo);
		tipoEnt = (arquivos[indice].tipoArq_LenNome & 0xF0) >> 4;
		++indice;
		if (tipoEnt != PRODOS_TA_DEL) {		// Excluir Deletados
			++c;
		}
		if (byteAtual >= 511) {
			if (!proximoBloco) {
				break;
			}
			lerBloco(proximoBloco, buffer);
			memcpy(&proximoBloco, buffer+2, sizeof(word));
			byteAtual = 4;
		}
	}
	numEntradas = indice; //  ? indice-1 : 0
}

/*****************************************************************************/
void listaDir(int bloco) {
	char  nome[16], dataCriacao[30] = "", dataModificacao[30] = "";
	char  strAux[6] = "";
	int   tipoEnt, tipoArquivo, blocosUsados;
	int   tamArquivo, byteAcesso, valorAuxiliar;
	int   i, c = 0;
	TEntradaArquivo arquivos2[PRODOS_MAXARQS];

	printf(" NOME            TIPO  BLOCOS     MODIFICADO        CRIADO       TAMANHO  AUX.\n");
	carregaDir(bloco);

	memcpy(&arquivos2, &arquivos, sizeof(TEntradaArquivo) * PRODOS_MAXARQS);
	memset(nome, 0, 16);

	for (i=0; i < numEntradas; i++) {
		tipoEnt = (arquivos2[i].tipoArq_LenNome & 0xF0) >> 4;
		if (tipoEnt != PRODOS_TA_DEL) {		// Arquivo não está deletado
			++c;
			memset(nome, 32, 15);
			strncpy(nome, (char *)arquivos2[i].nomeArquivo, arquivos2[i].tipoArq_LenNome & 0x0F);
			tipoArquivo   = arquivos2[i].tipoDeArquivo;
			blocosUsados  = arquivos2[i].blocosUsados;
			tamArquivo    = (arquivos2[i].fimDoArquivoHi << 16) + arquivos2[i].fimDoArquivoLo;
			byteAcesso    = arquivos2[i].byteDeAcesso;
			valorAuxiliar = arquivos2[i].valorAuxiliar;
			converteDataProDos(dataCriacao, arquivos2[i].dataCriacao, arquivos2[i].horaCriacao);
			converteDataProDos(dataModificacao, arquivos2[i].dataModificacao, arquivos2[i].horaModificacao);

			if (byteAcesso & PODESERESCRITO) {
				printf(" ");
			} else {
				printf("*");
			}
			/* if (tiposArquivo[tipoArquivo].temEndereco) { */
				sprintf(strAux,"$%.4X",valorAuxiliar);
			/* } else {
				sprintf(strAux,""); 
			} */
			printf("%s  %s %7d  %s  %s  %7d  %s\n", 
					nome, 
					tiposArquivo[tipoArquivo].texto,
					blocosUsados,
					dataModificacao,
					dataCriacao,
					tamArquivo,
					strAux
					);
		} // if tipoEnt != PRODOS_TA_DEL

	} // for i
	if (!c) {
		printf("\n <Sem arquivos>\n");
		return;
	}

}

/*****************************************************************************/
TEntradaArquivo* achaArquivo(char* nomeApple, int bloco) {
	int  tipoEnt, i;
	char nomeArquivo[240];

	carregaDir(bloco);
	verificaNomeApple(nomeApple);
	for (i=0; i < numEntradas; i++) {
		tipoEnt = (arquivos[i].tipoArq_LenNome & 0xF0) >> 4;
		if ((tipoEnt != PRODOS_TA_DEL))	{		// Arquivo não está deletado
			memset(nomeArquivo, 0, 16);
			strncpy(nomeArquivo, (char *)arquivos[i].nomeArquivo, arquivos[i].tipoArq_LenNome & 0x0F);
			if (!strcmp(nomeArquivo, nomeApple)) {
				return &arquivos[i];
			}
		}
	}
	return NULL;
}

/*****************************************************************************/
int achaBlocoDiretorio(char* caminho) {
	int  bloco = 2, i=0, p=0;
	char caminho2[240];
	char parte[16];
	int  TipoArq;
	TEntradaArquivo* entrada;

	strcpy(caminho2, caminho);
	if (!strcmp(caminho2, "/")) {
		return 2;
	}

	strcat(caminho2, "/");
	while(i < strlen(caminho2)) {
		while(caminho2[i++] != '/') ;
		memset(parte, 0, 16);
		strncpy(parte, caminho2+p, i-p-1);
		p = i;
		if (strlen(parte)) {
			entrada = achaArquivo(parte, bloco);
			if (!entrada) {
				return 0;
			}
			TipoArq = (entrada->tipoArq_LenNome & 0xF0) >> 4;
			if (TipoArq != PRODOS_TA_DIR) {
				return 0;
			}
			bloco = entrada->blocoArquivo;
		}
	}
	return bloco;
}

// Comandos:
/*****************************************************************************/
void create(int blocos, char *nomeVolume) {
	char buffer[512];
	int  i, tB;
	word  wData, wHora;

	verificaNomeApple(nomeVolume);

	memset(buffer, 0, 512);
	for (i = 0; i < blocos; i++) {
		escreverBloco(i, buffer);
	}

	// Grava setor de boot
	escreverBloco(0, prodosBoot0);
	escreverBloco(1, prodosBoot1);

	// Gera informações do volume
	memset(&volume, 0, sizeof(TEntradaDirVol));
	obtemDataProDos(&wData, &wHora);
	volume.tipoArq_LenNome  = (PRODOS_TA_CVOL << 4) | strlen(nomeVolume);
	strcpy((char *)volume.nome, nomeVolume);
	volume.dataDaCriacao          = wData;
	volume.horaDaCriacao          = wHora;
	volume.versaoProDos           = 0x23;
	volume.versaoMinima           = 0x00;
	volume.byteDeAcesso           = 0xC3;
	volume.tamanhoDaEntrada       = sizeof(TEntradaArquivo);
	volume.entradasPorBloco       = 0x0D;
	volume.entradasAtivas         = 0x00;
	volume.volBlocoBitMap         = 0x06;
	volume.volTotalBlocosDoVolume = blocos;

	// Preenche informações dos blocos 2,3,4,5 que
	// contém informaçõees das entradas dos arquivos
	buffer[2] = 3;
	memcpy(buffer+4, &volume, sizeof(TEntradaDirVol));
	escreverBloco(2, buffer);
	memset(buffer, 0, 512);
	buffer[0] = 2;
	buffer[2] = 4;
	escreverBloco(3, buffer);
	buffer[0] = 3;
	buffer[2] = 5;
	escreverBloco(4, buffer);
	buffer[0] = 4;
	buffer[2] = 0;
	escreverBloco(5, buffer);
	// Calcula quantos blocos são necessários para o bitMap dos
	// blocos ocupados e preenche
	tB = (blocos-1) / 4096;
	for (i = 0; i <= (6+tB); i++) {
		ocupaBloco(i);
	}
	for (i = (7+tB); i < blocos; i++) {
		liberaBloco(i);
	}

	for (i = 6; i <= (6+tB); i++) {
		escreverBloco(i, (char *)bitMap+((i-6)*512));
	}
	printf("Imagem criada\n");
}

/*****************************************************************************/
void dir(char* nomeDiretorio) {
	int  bloco;
	char nome[16];
	int  totalBlocos = (int)volume.volTotalBlocosDoVolume;

	memset(nome, 0, 16);
	strncpy(nome, (char *)volume.nome, volume.tipoArq_LenNome & 0x0F);
	printf("\nVolume /%s/\n", nome);

	bloco = achaBlocoDiretorio(nomeDiretorio);
	if (bloco) {
		printf("\nDiretorio: %s\n\n", nomeDiretorio);
		listaDir(bloco);

		printf("\n Blocos Livres: %d - Blocos Usados: %d - Total de Blocos: %d\n",
				retornaBlocosLivres(), 
				totalBlocos - retornaBlocosLivres(), 
				totalBlocos
				);
	} else {
		sair(ERR_DIRETORIONAOACHADO, "Diretorio nao achado");
	}
}

/*****************************************************************************/
void out(char *nomeApple, int processar) {
	char buffer[512], indices[1024];
	int  bloco = 2, t, x;
	int  tipoEnt, tamArquivo, tamBloco;
	char nomeArquivoSaida[240], nomeArquivoTemp[240];
	char nomeArquivo[240], nomeDiretorio[240];
	char *bufferTemp;
	FILE  *arqTemp;
	TEntradaArquivo* entrada;

	memset(nomeDiretorio, 0, sizeof(nomeDiretorio));
	memset(nomeArquivo, 0, sizeof(nomeArquivo));
	strcpy(buffer, "//");
	strcat(buffer, nomeApple);
	t = strlen(buffer);
	while(buffer[--t] != '/') ;
	strncpy(nomeDiretorio, buffer+1, t-1);
	strcpy(nomeArquivo, buffer+t+1);

	verificaNomeApple(nomeArquivo);

	if (strlen(nomeDiretorio)) {
		bloco = achaBlocoDiretorio(nomeDiretorio);
	}

	entrada = achaArquivo(nomeArquivo, bloco);
	if (entrada) {
		tipoEnt = (entrada->tipoArq_LenNome & 0xF0) >> 4;
		// Verifica se arquivo não é um diretório
		if (tipoEnt > PRODOS_TA_TREE) {
			sair(ERR_ARQUIVOEDIRETORIO, "Arquivo nao é um arquivo plano");
		}
		// Monta nome do arquivo de saída
		sprintf(nomeArquivoSaida, 
					"%s#%.2X%.4X",
					nomeArquivo,
					entrada->tipoDeArquivo,
					entrada->valorAuxiliar);
		strcpy(nomeArquivoTemp, nomeArquivoSaida);
		printf("Extraindo %s\n", nomeArquivoSaida);
		if (!(arqSaida = fopen(nomeArquivoSaida, "wb"))) {
			sair(ERR_ERROAOCRIARARQUIVO, "Erro ao criar arquivo de saida");
		}
		tamArquivo = (entrada->fimDoArquivoHi << 16) + entrada->fimDoArquivoLo;
	
		switch(tipoEnt) {
			case PRODOS_TA_SEED:		// arquivo "seedling" (usa somente um bloco)
				lerBloco(entrada->blocoArquivo, buffer);
				fwrite(buffer, tamArquivo, 1, arqSaida);
				fclose(arqSaida);
				arqSaida = NULL;
			break;

			case PRODOS_TA_SAPL:		// arquivo "sapling" (usa de 2 até 256 blocos)
				lerBloco(entrada->blocoArquivo, indices);
				for (t = 0; t < 256; t++) {
					bloco = (indices[t + 256] << 8) + indices[t];
					if (bloco != 0) {
						lerBloco(bloco, buffer);
						tamBloco = tamArquivo > 512 ? 512 : tamArquivo;
						fwrite(buffer, tamBloco, 1, arqSaida);
						tamArquivo -= tamBloco;
					} // if bloco != 0
				} // for t
				fclose(arqSaida);
				arqSaida = NULL;
			break;

			case PRODOS_TA_TREE:		// arquivo "tree" (usa de 257 blocos até 32768 blocos)
				lerBloco(entrada->blocoArquivo, indices);
				for (t = 0; t < 256; t++) {
					bloco = (indices[t + 256] << 8) + indices[t];
					if (bloco != 0) {
						lerBloco(bloco, indices + 512);
						for (x = 0; x < 256; x++) {
							bloco = (indices[512 + x + 256] << 8) + indices[512 + x];
							if (bloco != 0) {
								lerBloco(bloco, buffer);
								tamBloco = tamArquivo > 512 ? 512 : tamArquivo;
								fwrite(buffer, tamBloco, 1, arqSaida);
								tamArquivo -= tamBloco;
							} // if bloco != 0
						} // for x
					} // if bloco != 0
				} // for t
				fclose(arqSaida);
				arqSaida = NULL;
			break;
		}
		if (processar) {
			switch (entrada->tipoDeArquivo) {
				int  i, t;
				size_t tamArquivo;
				TApplesoftLinha applesoftLinha;
				char linha[240];

				case PRODOS_ARQBASIC:
					//strcat(nomeArquivoTemp, ".BAS");
					if (!(arqSaida = fopen(nomeArquivoSaida, "rb"))) {
						sair(ERR_ERROAOCRIARARQUIVO, "Erro ao abrir arquivo de saida");
					}
				    fseek (arqSaida, 0, SEEK_END);
					tamArquivo = ftell(arqSaida);
				    fseek (arqSaida, 0, SEEK_SET);
					bufferTemp = malloc(tamArquivo);
					if (!bufferTemp) {
						sair(ERR_SEMRECURSOS, "Falta de recursos do sistema");
					}
					fread(bufferTemp, tamArquivo, 1, arqSaida);
					fclose(arqSaida);
					arqSaida = NULL;

					if (!(arqTemp = fopen(nomeArquivoTemp, "wb"))) {
						sair(ERR_ERROAOCRIARARQUIVO, "Erro ao criar arquivo de saida");
					}
					i = 0;
					while (i < tamArquivo) {
						memcpy(&applesoftLinha, bufferTemp + i, sizeof(TApplesoftLinha));
						t = i + sizeof(TApplesoftLinha);
						if (applesoftLinha.proximoEnd == 0) {
							break;
						} else {
							i = applesoftLinha.proximoEnd - APPLESOFTENDBASE;
							sprintf(linha, "%5d ", applesoftLinha.numeroLinha);
							while(bufferTemp[t] != 0) {
								if (bufferTemp[t] & 0x80) {
									strcat(linha, applesoftTokens[bufferTemp[t] & 0x7F]);
								} else {
									strncat(linha, &bufferTemp[t], 1);
								}
								++t;
							}
							strcat(linha, "\n");
							fwrite(linha, strlen(linha), 1, arqTemp);
						} // if proximoend
					} // while i<Tam
				break;

				case PRODOS_ARQTEXTO:
					if (!(arqSaida = fopen(nomeArquivoSaida, "rb"))) {
						sair(ERR_ERROAOCRIARARQUIVO, "Erro ao abrir arquivo de saida");
					}
				    fseek (arqSaida, 0, SEEK_END);
					tamArquivo = ftell(arqSaida);
				    fseek (arqSaida, 0, SEEK_SET);
					bufferTemp = malloc(tamArquivo);
					if (!bufferTemp) {
						sair(ERR_SEMRECURSOS, "Falta de recursos do sistema");
					}
					fread(bufferTemp, tamArquivo, 1, arqSaida);
					fclose(arqSaida);
					arqSaida = NULL;

					if (!(arqTemp = fopen(nomeArquivoTemp, "wb"))) {
						sair(ERR_ERROAOCRIARARQUIVO, "Erro ao criar arquivo de saida");
					}
					i = 0;
					while (i < tamArquivo) {
						if ((bufferTemp[i] > 31 && bufferTemp[i] < 128) ||
								bufferTemp[i] == 0x0D || bufferTemp[i] == 0x0A) {
							fwrite(&bufferTemp[i], 1, 1, arqTemp);
						}
						if (bufferTemp[i] > 159 && bufferTemp[i] < 256) {
							bufferTemp[i] -= 128;
							fwrite(&bufferTemp[i], 1, 1, arqTemp);
						}
						++i;
					}
				break;
			} // switch
		} // if processar
	} else {
		sair(ERR_ARQUIVONAOACHADO, "Arquivo nao achado!");
	}
}

/*****************************************************************************/
void del(char *nomeApple, int forcar) {
	char buffer[512];
	int  bloco = 2, numEntrada = -1;
	int  tipoEnt;
	int  i, t;
	char nomeArquivo[240], diretorio[240];

	nomeApple[15] = '\0';
	for (i=0; i < strlen(nomeApple); i++) {
	  nomeApple[i] = toupper(nomeApple[i]);
	}

	memset(diretorio, 0, sizeof(diretorio));
	memset(nomeArquivo, 0, sizeof(nomeArquivo));
	strcpy(buffer, "//");
	strcat(buffer, nomeApple);
	t = strlen(buffer);
	while(buffer[--t] != '/') ;
	strncpy(diretorio, buffer+1, t-1);
	strcpy(nomeArquivo, buffer+t+1);

	if (strlen(diretorio)) {
		bloco = achaBlocoDiretorio(diretorio);
	}

	if (!bloco) {
		sair(ERR_DIRETORIONAOACHADO, "Diretorio Nao Achado");
	}

	carregaDir(bloco);

	for (i = 0; i < numEntradas; i++) {
		tipoEnt = (arquivos[i].tipoArq_LenNome & 0xF0) >> 4;
		arquivos[i].nomeArquivo[arquivos[i].tipoArq_LenNome & 0x0F] = 0;
		if (tipoEnt != PRODOS_TA_DEL) {
			if (!strcmp(nomeArquivo, (char *)arquivos[i].nomeArquivo)) {
				if (tipoEnt == PRODOS_TA_DIR) {
					sair(ERR_ARQUIVOEDIRETORIO, "Arquivo é um diretorio");
				}
				numEntrada = i;
				removeArquivo(bloco, i);
				break;
			}
		}
	}
	if (numEntrada == -1 && !forcar) {
		sair(ERR_ARQUIVONAOACHADO, "Arquivo nao achado");
	} else {
		printf("Arquivo Deletado: %s\n", nomeArquivo);
	}
}

/*****************************************************************************/
void in(char *tipo, char *nomeApple, char *nomeArquivoEntrada, int forcar) {
	char buffer[512], indices[1024];
	int  nTipo, bloco = 2, blocos[2];
	int  numEntrada = -1, valorAuxiliar = 0;
	int  tipoEnt, tamArquivo, byteAtual = 0;
	int  i, t, tipoBin = -1, blocosDeIndice = 0;
	char nomeArquivo[240], nomeDiretorio[240], temp[240];
	char *bufferTemp;
	word  wData, wHora;

	// Verifica se é necessário "scanear" o nome do arquivo pra
	// descobrir o Nome_Apple e/ou tipo de arquivo
	if (!tipo || !nomeApple) {
		char *pos = temp;
		strcpy(temp, nomeArquivoEntrada);

		if (strrchr(pos, '#')) {
			pos = strrchr(pos, '#');
			sscanf(pos+1,"%2X%4X", &nTipo, &valorAuxiliar);
			pos[0] = 0;
			tipo = pos + 1;
			pos = temp;
			strncpy(tipo, tiposArquivo[nTipo].texto, 4);
			if (!nomeApple) {
				nomeApple = pos;
				i = strlen(nomeApple);
				while(i > 0) {
					if (pos[i--] == '\\') {
						nomeApple = pos + i + 2;
						break;
					}
				}
			}
		}
	}
	if (!nomeApple) {
		sair(ERR_ERROLINHACOMANDO, "Faltando nome apple");
	}
	if (strlen(tipo) < 3) {
		sair(ERR_ERROLINHACOMANDO, "Tipo de arquivo deve conter 3 letras");
	}
	if (tipo[3] == '.') {
		if (strlen(tipo) < 5) {
			sair(ERR_ERROLINHACOMANDO, "Faltando valor auxiliar");
		}
		if (tipo[4] == '$') {
			sscanf(tipo+5,"%x",&valorAuxiliar);
		} else {
			valorAuxiliar = atoi(tipo+4);
		}
		tipo[3] = '\0';
	}
	for (i = 0; i < 3; i++) {
		tipo[i] = toupper(tipo[i]);
	}

	if (0 == retornaBlocosLivres()) {
		sair(ERR_DISCOCHEIO, "Disco esta cheio");
	}

	memset(nomeDiretorio, 0, sizeof(nomeDiretorio));
	memset(nomeArquivo, 0, sizeof(nomeArquivo));
	strcpy(buffer, "//");
	strcat(buffer, nomeApple);
	t = strlen(buffer);
	while(buffer[--t] != '/') ;
	strncpy(nomeDiretorio, buffer+1, t-1);
	strcpy(nomeArquivo, buffer+t+1);

	verificaNomeApple(nomeArquivo);

	if (strlen(nomeDiretorio)) {
		bloco = achaBlocoDiretorio(nomeDiretorio);
	}

	if (0 == bloco) {
		sair(ERR_DIRETORIONAOACHADO, "Diretorio Nao Achado");
	}

	carregaDir(bloco);

	if (diretorio.entradasAtivas == PRODOS_MAXARQS) {
		sair(ERR_DIRETORIOCHEIO, "Diretorio esta cheio");
	}

	for (i=0; i < numEntradas; i++) {
		tipoEnt = (arquivos[i].tipoArq_LenNome & 0xF0) >> 4;
		arquivos[i].nomeArquivo[arquivos[i].tipoArq_LenNome & 0x0F] = 0;
		if (tipoEnt != PRODOS_TA_DEL) {
			if (!strcmp(nomeArquivo, (char *)arquivos[i].nomeArquivo)) {
				if (tipoEnt == PRODOS_TA_DIR) {
					sair(ERR_ARQUIVOEDIRETORIO, "Arquivo e' um diretorio");
				}
				if (forcar) {
					del(nomeApple, 1);
				} else {
					sair(ERR_ARQUIVOJAEXISTE, "Arquivo ja existe");
				}
			}
		}
	}
	if (numEntrada == -1) {
		for (i=0; i <= numEntradas; i++) {
			tipoEnt = (arquivos[i].tipoArq_LenNome & 0xF0) >> 4;
			if ((tipoEnt == PRODOS_TA_DEL))	{		// Arquivo está deletado
				numEntrada = i;
				break;
			}
		}
	}

	for (i = 0; i < 256; i++) {
		if (i != 0x0F) {	// Pular Verificação de diretório
			if (!strcmp(tipo, tiposArquivo[i].texto)) {
				tipoBin = i;
				break;
			}
		}
	}
	if (tipoBin == -1) {
		int i;

		fprintf(stderr, "Tipo de arquivo nao achado\n\n");
		fprintf(stderr, "Lista de tipos de arquivo validas:\n");

		for (i = 0; i < 256; i++) {
			if (!(i % 16)) {
				printf("\n");
			}
			fprintf(stderr, "%s ", tiposArquivo[i].texto);
		} // for i
		sair(ERR_ERROLINHACOMANDO, "");
	} // if tipoBin == -1

	if (!(arqSaida = fopen(nomeArquivoEntrada, "rb"))) {
		sair(ERR_ERROAOCRIARARQUIVO, "Erro ao abrir arquivo");
	}

	fseek(arqSaida, 0, SEEK_END);
	tamArquivo = ftell(arqSaida);
	fseek(arqSaida, 0, SEEK_SET);

	if (tamArquivo <= 512) {
		blocosDeIndice = 0;
	} else if (tamArquivo <= 131072) {
		blocosDeIndice = 1;
	} else {
		blocosDeIndice = 1 + (tamArquivo / 131072);
	}
/*
	if (tamArquivo == 0)
		sair(ERR_ARQUIVONAOACHADO, "Arquivo vazio");
*/
	if (tamArquivo >= (retornaBlocosLivres()-blocosDeIndice) * 512)
		sair(ERR_DISCOCHEIO, "Arquivo muito grande");

	bufferTemp = malloc(tamArquivo+512);
	if (!bufferTemp) {
		sair(ERR_SEMRECURSOS, "Falta de recursos do sistema");
	}

	fread(bufferTemp, tamArquivo, 1, arqSaida);
	fclose(arqSaida);

	obtemDataProDos(&wData, &wHora);
	tipoEnt = (blocosDeIndice == 0 ? PRODOS_TA_SEED :
				blocosDeIndice == 1 ? PRODOS_TA_SAPL : PRODOS_TA_TREE); 
	arquivos[numEntrada].tipoArq_LenNome = (tipoEnt << 4) | strlen(nomeArquivo);
	strcpy((char *)arquivos[numEntrada].nomeArquivo, nomeArquivo);
	arquivos[numEntrada].tipoDeArquivo = tipoBin;
	arquivos[numEntrada].blocoArquivo = achaBlocoLivre();
	arquivos[numEntrada].blocosUsados = (tamArquivo-1) / 512 + blocosDeIndice + 1;
	arquivos[numEntrada].fimDoArquivoLo =  tamArquivo & 0x0000FFFF;
	arquivos[numEntrada].fimDoArquivoHi = (tamArquivo & 0xFFFF0000) >> 16;
	arquivos[numEntrada].dataCriacao = wData;
	arquivos[numEntrada].horaCriacao = wHora;
	arquivos[numEntrada].versaoProDos = diretorio.versaoProDos;
	arquivos[numEntrada].versaoMinima = diretorio.versaoMinima;
	arquivos[numEntrada].byteDeAcesso = 0xC3;
	arquivos[numEntrada].valorAuxiliar = valorAuxiliar;
	arquivos[numEntrada].dataModificacao = wData;
	arquivos[numEntrada].horaModificacao = wHora;
	arquivos[numEntrada].blocoCabecalho = bloco;

	diretorio.entradasAtivas++;
	salvaDir(bloco);

	bloco = arquivos[numEntrada].blocoArquivo;
	ocupaBloco(bloco);

	switch(tipoEnt) {
		case PRODOS_TA_SEED:
			escreverBloco(bloco, bufferTemp);
		break;

		case PRODOS_TA_SAPL:
			memset(indices, 0, 1024);
			blocos[0] = bloco;
			for (i = 0; i < 256; i++) {
				bloco = achaBlocoLivre();
				ocupaBloco(bloco);
				indices[i] = bloco & 0x00FF;
				indices[i+256] = (bloco & 0xFF00) >> 8;
				escreverBloco(bloco, bufferTemp + byteAtual);
				byteAtual += 512;
				if (byteAtual >= tamArquivo) {
					break;
				}
			}
			escreverBloco(blocos[0], indices);
		break;

		case PRODOS_TA_TREE:
			memset(indices, 0, 1024);
			blocos[0] = bloco;
			for (t = 0; t < 256; t++) {
				bloco = achaBlocoLivre();
				ocupaBloco(bloco);
				blocos[1] = bloco;
				indices[t] = bloco & 0x00FF;
				indices[t+256] = (bloco & 0xFF00) >> 8;
				for (i = 0; i < 256; i++) {
					bloco = achaBlocoLivre();
					ocupaBloco(bloco);
					indices[i+512] = bloco & 0x00FF;
					indices[i+256+512] = (bloco & 0xFF00) >> 8;
					escreverBloco(bloco, bufferTemp+byteAtual);
					byteAtual += 512;
					if (byteAtual >= tamArquivo) {
						break;
					}
				} // for i
				escreverBloco(blocos[1], indices+512);
				if (byteAtual >= tamArquivo) {
					break;
				}
			} // for t
			escreverBloco(blocos[0], indices);
		break;
	}
	free(bufferTemp);
	escreverBloco(volume.volBlocoBitMap, (char *)bitMap);
	printf("Arquivo Inserido: %s\n", nomeArquivo);

}

/*****************************************************************************/
void make(char *tipo, char *nomeApple, int forcar) {
	char buffer[512];
	int  bloco = 2, BlocoParente;
	int  numEntrada = -1, valorAuxiliar = 0;
	int  tipoEnt;
	int  i, t, tipoBin = -1;
	char nomeArquivo[240], nomeDiretorio[240];
	word  wData, wHora;

	if (strlen(tipo) < 3) {
		sair(ERR_ERROLINHACOMANDO, "Tipo de arquivo deve conter 3 letras");
	}
	if (tipo[3] == '.') {
		if (strlen(tipo) < 5) {
			sair(ERR_ERROLINHACOMANDO, "Faltando valor auxiliar");
		}
		if (tipo[4] == '$') {
			sscanf(tipo+5,"%x",&valorAuxiliar);
		} else {
			valorAuxiliar = atoi(tipo+4);
		}
		tipo[3] = '\0';
	}
	for (i=0; i < 3; i++) {
		tipo[i] = toupper(tipo[i]);
	}

	if (!retornaBlocosLivres()) {
		sair(ERR_DISCOCHEIO, "Disco esta cheio");
	}

	memset(nomeDiretorio, 0, sizeof(nomeDiretorio));
	memset(nomeArquivo, 0, sizeof(nomeArquivo));
	strcpy(buffer, "//");
	strcat(buffer, nomeApple);
	t = strlen(buffer);
	while(buffer[--t] != '/') ;
	strncpy(nomeDiretorio, buffer+1, t-1);
	strcpy(nomeArquivo, buffer+t+1);

	verificaNomeApple(nomeArquivo);

	if (strlen(nomeDiretorio)) {
		bloco = achaBlocoDiretorio(nomeDiretorio);
	}

	if (!bloco) {
		sair(ERR_DIRETORIONAOACHADO, "Diretorio Nao Achado");
	}

	carregaDir(bloco);
	BlocoParente = bloco;

	if (diretorio.entradasAtivas == PRODOS_MAXARQS) {
		sair(ERR_DIRETORIOCHEIO, "Diretorio esta cheio");
	}

	for (i=0; i < numEntradas; i++) {
		tipoEnt = (arquivos[i].tipoArq_LenNome & 0xF0) >> 4;
		arquivos[i].nomeArquivo[arquivos[i].tipoArq_LenNome & 0x0F] = 0;
		if (tipoEnt != PRODOS_TA_DEL) {
			if (!strcmp(nomeArquivo, (char *)arquivos[i].nomeArquivo)) {
				if (forcar) {
					return;
				} else {
					sair(ERR_ARQUIVOJAEXISTE, "Arquivo ja existe");
				}
			}
		}
	}
	if (numEntrada == -1) {
		for (i = 0; i <= numEntradas; i++) {
			tipoEnt = (arquivos[i].tipoArq_LenNome & 0xF0) >> 4;
			if ((tipoEnt == PRODOS_TA_DEL))	{			// Arquivo está deletado
				numEntrada = i;
				break;
			}
		}
	}

	for (i = 0; i < 256; i++) {
		if (!strcmp(tipo, tiposArquivo[i].texto)) {
			tipoBin = i;
			break;
		}
	}
	if (tipoBin == -1) {
		int i;

		fprintf(stderr, "Tipo de arquivo nao achado\n\n");
		fprintf(stderr, "Lista de tipos de arquivo validas:\n");

		for (i = 0; i < 256; i++) {
			if (!(i % 16)) {
				printf("\n");
			}
			fprintf(stderr, "%s ", tiposArquivo[i].texto);
		} // for i
		sair(ERR_ERROLINHACOMANDO, "");
	} // if tipoBin == -1

	obtemDataProDos(&wData, &wHora);
	tipoEnt = (tipoBin == PRODOS_ARQDIR ? PRODOS_TA_DIR : PRODOS_TA_SEED); 
	arquivos[numEntrada].tipoArq_LenNome = (tipoEnt << 4) | strlen(nomeArquivo);
	strcpy((char *)arquivos[numEntrada].nomeArquivo, nomeArquivo);
	arquivos[numEntrada].tipoDeArquivo = tipoBin;
	arquivos[numEntrada].blocoArquivo = achaBlocoLivre();
	arquivos[numEntrada].blocosUsados = 1;
	arquivos[numEntrada].fimDoArquivoLo = 512;
	arquivos[numEntrada].fimDoArquivoHi = 0;
	arquivos[numEntrada].dataCriacao = wData;
	arquivos[numEntrada].horaCriacao = wHora;
	arquivos[numEntrada].versaoProDos = diretorio.versaoProDos;
	arquivos[numEntrada].versaoMinima = diretorio.versaoMinima;
	arquivos[numEntrada].byteDeAcesso = 0xE3;
	arquivos[numEntrada].valorAuxiliar = valorAuxiliar;
	arquivos[numEntrada].dataModificacao = wData;
	arquivos[numEntrada].horaModificacao = wHora;
	arquivos[numEntrada].blocoCabecalho = bloco;

	diretorio.entradasAtivas++;
	salvaDir(bloco);

	bloco = arquivos[numEntrada].blocoArquivo;
	ocupaBloco(bloco);

	if (tipoBin == PRODOS_ARQDIR) {
		TEntradaDirVol diretorioC;

		memset(&diretorioC, 0, sizeof(TEntradaDirVol));

		diretorioC.tipoArq_LenNome = (PRODOS_TA_CDIR << 4) | strlen(nomeArquivo);
		strcpy((char *)diretorioC.nome, nomeArquivo);
		memset(&diretorioC.reservado, 0, 8);
		diretorioC.reservado[0]     = 0x75;
		diretorioC.dataDaCriacao    = wData;
		diretorioC.horaDaCriacao    = wHora;
		diretorioC.versaoProDos     = diretorio.versaoProDos;
		diretorioC.versaoMinima     = diretorio.versaoMinima;
		diretorioC.byteDeAcesso     = 0xC3;
		diretorioC.tamanhoDaEntrada = sizeof(TEntradaArquivo);
		diretorioC.entradasPorBloco = 13;
		diretorioC.entradasAtivas   = 0;
		diretorioC.dirBlocoParente = BlocoParente;
		diretorioC.dirNumTamEntradaParente = ((byte)sizeof(TEntradaArquivo) << 8) | (byte)(numEntrada+2);
		memset(buffer, 0, 512);
		memcpy(buffer+4, &diretorioC, sizeof(TEntradaDirVol));
		escreverBloco(bloco, buffer);
		printf("Diretorio Criado: %s\n", nomeArquivo);
	} else {
		printf("Arquivo Criado: %s\n", nomeArquivo);
	}

	escreverBloco(volume.volBlocoBitMap, (char *)bitMap);
}

/*****************************************************************************/
void mostraUso() {
	fprintf(stderr, "\nProDos.exe - utilitario para manipulacao de imagens de disco ProDos\n");
	fprintf(stderr, "Versao " VERSION "\n\n");
	fprintf(stderr, "  Uso: ProDos <nome da imagem do disco> <comandos e opcoes>\n\n");
	fprintf(stderr, "  Comandos:\n");
	fprintf(stderr, "    -c <-1 | -s <blocos>> <-v <Volume>>\n");
	fprintf(stderr, "       Cria uma nova imagem\n");
	fprintf(stderr, "    -l [diretorio]\n");
	fprintf(stderr, "       Exibe o catalogo do disco\n");
	fprintf(stderr, "    -i [-f] [-t <Tipo[.aux]>] <-a <Nome_Apple2>> <Arquivo>\n");
	fprintf(stderr, "       Embute um arquivo no disco\n");
	fprintf(stderr, "    -o [-p] <-a <Nome_Apple2>>\n");
	fprintf(stderr, "       Extrai um arquivo do disco\n");
	fprintf(stderr, "    -d [-f] <-a <Nome_Apple2>>\n");
	fprintf(stderr, "       Deleta um arquivo do disco\n");
	fprintf(stderr, "    -m [-f] <-t <Tipo[.aux]>> <-a <Nome_Apple2>>\n");
	fprintf(stderr, "       Cria um arquivo ou diretorio do disco\n\n");
	fprintf(stderr, "  Opcoes:\n");
	fprintf(stderr, "    -1               = Criar uma imagem de 140KB\n");
	fprintf(stderr, "    -s <blocos>      = Criar uma imagem de <blocos> blocos\n");
	fprintf(stderr, "    -v <Nome_Volume> = Nome do volume no formato do Apple2\n");
	fprintf(stderr, "    -a <Nome_Apple2> = Nome do arquivo no formato do Apple2\n");
	fprintf(stderr, "    -t <Tipo[.aux]>  = Tipo do arquivo com o valor auxiliar\n");
	fprintf(stderr, "    -p               = Processar arquivo de saida\n");
	fprintf(stderr, "    -f               = Forcar\n");
	exit(ERR_SEMERROS);
}

/*****************************************************************************/
int main (int argc, char *argv[]) {
	char *nomeImagem = NULL;
	char *nomeApple = NULL;
	char *nomeArquivo  = NULL;
	char *tipo = NULL;
	int  c = 2, tamImagem = 0;
	int  comando = 0;
	int  processar = 0;
	int  somenteLeitura = 0;
	int  forcar = 0;

	if (argc < 3) {
		mostraUso();
	}

	nomeImagem = argv[1];

	// Interpreta linha de comando
	while (c < (int)argc) {
		// É uma opção
		if (argv[c][0] == '-' || argv[c][0] == '/') {
			// Opções sem parâmetros
			switch(argv[c][1]) {
				case 'c':
					comando = CMD_CREATE;
				break;

				case 'l':
					comando = CMD_DIR;
				break;

				case 'i':
					comando = CMD_IN;
				break;

				case 'o':
					comando = CMD_OUT;
				break;

				case 'd':
					comando = CMD_DELETE;
				break;

				case 'm':
					comando = CMD_MAKE;
				break;

				case 'p':
					processar = 1;
				break;

				case 'f':
					forcar = 1;
				break;

				case '1':
					tamImagem = 280;		// 280 blocos = 140KB
				break;

				default:
					// Opções com parâmetros
					if (c+1 == (int)argc) {
						fprintf(stderr, "Falta parametro para a opcao %s", argv[c]);
						return 1;
					}
					switch(argv[c][1]) {
						case 's':
							++c;
							tamImagem = MIN(65535, MAX(10, atoi(argv[c])));
						break;

						case 'a':
						case 'v':
							++c;
							nomeApple = argv[c];
						break;

						case 't':
							++c;
							tipo = argv[c];
						break;

						default:
							fprintf(stderr, "Opcao invalida: %s\n", argv[c]);
							return ERR_ERROLINHACOMANDO;
						break;
					} // switch
				break;
			} // switch
		} else {
			nomeArquivo = argv[c];
		}
		c++;
	}

	if (comando != CMD_CREATE) {
		if (!(arqEntrada = fopen(nomeImagem, "rb+"))) {
			if (!(arqEntrada = fopen(nomeImagem, "rb"))) {
				sair(ERR_ERROAOCRIARARQUIVO, "Erro ao abrir arquivo");
			}
			somenteLeitura = 1;
		}
		verificaVolume();
	}

	switch(comando) {
		case CMD_CREATE:
			if (!nomeApple || !tamImagem) {
				mostraUso();
			}
			if (!(arqEntrada = fopen(nomeImagem, "wb+"))) {
				sair(ERR_ERROAOCRIARARQUIVO, "Erro ao criar arquivo");
			}
			if (tamImagem == 280) {		// 280 blocos = 140KB
				eImagemDSK = 1;
			}
			create(tamImagem, nomeApple);
		break;

		case CMD_DIR:
			if (nomeArquivo) {
				dir(nomeArquivo);
			} else {
				dir("/");
			}
		break;

		case CMD_IN:
			if (somenteLeitura) {
				sair(ERR_ERROSOMENTELEITURA, "Arquivo somente leitura");
			}
			if (!nomeArquivo) {
				mostraUso();
			}
			in(tipo, nomeApple, nomeArquivo, forcar);
		break;

		case CMD_OUT:
			if (!nomeApple) {
				mostraUso();
			}
			out(nomeApple, processar);
		break;

		case CMD_DELETE:
			if (somenteLeitura) {
				sair(ERR_ERROSOMENTELEITURA, "Arquivo somente leitura");
			}
			if (!nomeApple) {
				mostraUso();
			}
			del(nomeApple, forcar);
		break;

		case CMD_MAKE:
			if (somenteLeitura) {
				sair(ERR_ERROSOMENTELEITURA, "Arquivo somente leitura");
			}
			if (!nomeApple || !tipo) {
				mostraUso();
			}
			make(tipo, nomeApple, forcar);
		break;

		default:
			sair(ERR_FALTACOMANDO, "Erro: Falta Comando!");
		break;
	}

	if (arqEntrada) {
		fclose(arqEntrada);
	}
	if (arqSaida) {
		fclose(arqSaida);
	}

	return ERR_SEMERROS;
}
