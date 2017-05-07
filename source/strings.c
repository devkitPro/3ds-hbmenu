#include "strings.h"

#define STR_JP(_str) [CFG_LANGUAGE_JP] = _str
#define STR_EN(_str) [CFG_LANGUAGE_EN] = _str
#define STR_FR(_str) [CFG_LANGUAGE_FR] = _str
#define STR_DE(_str) [CFG_LANGUAGE_DE] = _str
#define STR_IT(_str) [CFG_LANGUAGE_IT] = _str
#define STR_ES(_str) [CFG_LANGUAGE_ES] = _str
#define STR_ZH(_str) [CFG_LANGUAGE_ZH] = _str
#define STR_KO(_str) [CFG_LANGUAGE_KO] = _str
#define STR_NL(_str) [CFG_LANGUAGE_NL] = _str
#define STR_PT(_str) [CFG_LANGUAGE_PT] = _str
#define STR_RU(_str) [CFG_LANGUAGE_RU] = _str
#define STR_TW(_str) [CFG_LANGUAGE_TW] = _str

const char* const g_strings[StrId_Max][16] =
{
	[StrId_Loading] =
	{
		STR_EN("Loading…"),
		STR_ES("Cargando…"),
		STR_DE("Lade…"),
		STR_FR("Chargement…"),
	},

	[StrId_Directory] =
	{
		STR_EN("Directory"),
		STR_ES("Carpeta"),
		STR_DE("Verzeichnis"),
		STR_FR("Dossier"),
	},

	[StrId_DefaultLongTitle] =
	{
		STR_EN("Homebrew application"),
		STR_ES("Aplicación casera"),
		STR_DE("Homebrew-Anwendung"),
		STR_FR("Application homebrew"),
	},

	[StrId_DefaultPublisher] =
	{
		STR_EN("Unknown author"),
		STR_ES("Autor desconocido"),
		STR_DE("Unbekannter Autor"),
		STR_FR("Auteur inconnu"),
	},

	[StrId_IOError] =
	{
		STR_EN("I/O Error"),
		STR_ES("Error de E/S"),
		STR_DE("IO-Fehler"),
		STR_FR("Erreur d'E/S"),
	},

	[StrId_CouldNotOpenFile] =
	{
		STR_EN("Could not open file:\n%s"),
		STR_ES("No se pudo abrir el archivo:\n%s"),
		STR_DE("Konnte Datei \"%s\" nicht öffnen."),
		STR_FR("Impossible d'ouvrir le fichier :\n%s")
	},

	[StrId_NoAppsFound_Title] =
	{
		STR_EN("No applications found"),
		STR_ES("No hay aplicaciones"),
		STR_DE("Keine Anwendungen gefunden"),
		STR_FR("Aucune application trouvée"),
	},

	[StrId_NoAppsFound_Msg] =
	{
		STR_EN(
			"No applications could be found on the SD card.\n"
			"Make sure a folder named /3ds exists in the\n"
			"root of the SD card and it contains applications.\n"
		),
		STR_ES(
			"No se han podido encontrar aplicaciones en la\n"
			"tarjeta SD. Compruebe que haya una carpeta\n"
			"llamada /3ds y que contenga aplicaciones.\n"
		),
		STR_DE(
		        "Auf der SD-Karte wurden keine Anwendungen\n"
			"gefunden. Stelle sicher, dass ein Verzeichnis\n"
			"names /3ds im Wurzelverzeichnis der SD-Karte\n"
			"existiert und Anwendungen enthält!"
		),
		STR_FR(
			"Aucune application n'a été trouvée sur la carte\n"
			"SD. Veillez à ce qu'un dossier intitulé /3ds\n"
			"existe à la racine de la carte SD et à ce qu'il\n"
			"contienne des applications."
		),
	},

	[StrId_Reboot] =
	{
		STR_EN(
			"Returning to \xEE\x81\xB3HOME is not available.\n"
			"You're about to reboot your console.\n\n"
			"  \xEE\x80\x80 Reboot\n"
			"  \xEE\x80\x81 Cancel"
		),
		STR_ES(
			"Volver a \xEE\x81\xB3HOME no está disponible.\n"
			"Está a punto de reiniciar su consola.\n\n"
			"  \xEE\x80\x80 Reiniciar\n"
			"  \xEE\x80\x81 Cancelar"
		),
		STR_DE(
		        "Rückkehr zu \xEE\x81\xB3HOME nicht verfügbar.\n"
			"Deine Konsole wird neu gestartet.\n\n"
			"  \xEE\x80\x80 Neu starten\n"
			"  \xEE\x80\x81 Abbrechen"	
		),
		STR_FR(
			"Retour au menu \xEE\x81\xB3HOME indisponible.\n"
			"Vous êtes sur le point de redémarrer votre\n"
			"console.\n\n"
			"  \xEE\x80\x80 Redémarrer\n"
			"  \xEE\x80\x81 Annuler"
		),
	},

	[StrId_ReturnToHome] =
	{
		STR_EN(
			"You're about to return to \xEE\x81\xB3HOME.\n\n"
			"  \xEE\x80\x80 Return\n"
			"  \xEE\x80\x81 Cancel\n"
			"  \xEE\x80\x82 Reboot"
		),
		STR_ES(
			"Está a punto de volver a \xEE\x81\xB3HOME.\n\n"
			"  \xEE\x80\x80 Volver\n"
			"  \xEE\x80\x81 Cancelar\n"
			"  \xEE\x80\x82 Reiniciar"
		),
	 	STR_DE(
			"Rückkehr zum \xEE\x81\xB3HOME-Menü.\n\n"
			"  \xEE\x80\x80 Fortfahren\n"
			"  \xEE\x80\x81 Abbrechen\n"
			"  \xEE\x80\x82 Konsole neustarten"
		),
		STR_FR(
			"Retour au menu \xEE\x81\xB3HOME.\n\n"
			"  \xEE\x80\x80 Continuer\n"
			"  \xEE\x80\x81 Annuler\n"
			"  \xEE\x80\x82 Redémarrer"
		),
	},

	[StrId_TitleSelector] =
	{
		STR_EN("Title selector"),
		STR_ES("Selector de título"),
		STR_DE("Titel-Selektor"),
		STR_DE("Sélecteur de titre"),
	},

	[StrId_ErrorReadingTitleMetadata] =
	{
		STR_EN("Error reading title metadata.\n%08lX%08lX@%d"),
		STR_ES("Error leyendo los metadatos de los títulos.\n%08lX%08lX@%d"),
		STR_DE("Fehler beim lesen der Titel-Metadaten.\n%08lX%08lX@%d"),
		STR_FR(
			"Erreur lors de la lecture des métadonnées\n"
			"de titre.\n%08lX%08lX@%d"
		),
	},

	[StrId_NoTitlesFound] =
	{
		STR_EN("No titles could be detected."),
		STR_ES("No se han podido detectar títulos."),
		STR_DE("Keine Titel gefunden."),
		STR_FR("Aucun titre trouvé"),
	},

	[StrId_SelectTitle] =
	{
		STR_EN(
			"Please select a target title.\n\n"
			"  \xEE\x80\x80 Select\n"
			"  \xEE\x80\x81 Cancel"
		),
		STR_ES(
			"Elija el título de destino.\n\n"
			"  \xEE\x80\x80 Seleccionar\n"
			"  \xEE\x80\x81 Cancelar"
		),
		STR_DE(
			"Bitte wähle den Ziel-Titel aus.\n\n"
			"  \xEE\x80\x80 Auswählen\n"
			"  \xEE\x80\x81 Abbrechen"
		),
		STR_FR(
			"Veuillez sélectionner un titre de destination.\n\n"
			"  \xEE\x80\x80 Sélectionner\n"
			"  \xEE\x80\x81 Annuler"
		),
	},

	[StrId_NoTargetTitleSupport] =
	{
		STR_EN(
			"This homebrew exploit does not have support\n"
			"for launching applications under target titles.\n"
			"Please use a different exploit."
		),
		STR_ES(
			"Este exploit de homebrew no tiene soporte para\n"
			"ejecutar aplicaciones bajo títulos de destino.\n"
			"Use otro exploit diferente."
		),
	 	STR_DE(
			"Dieser Homebrew-Exploit unterstützt das starten\n"
			"von Anwendungen unter Ziel-Titeln nicht.\n"
			"Bitte verwende einen anderen Exploit."
		),
		STR_FR(
			"Cet exploit homebrew ne permet pas de lancer\n"
			"des applications sous des titres précis.\n"
			"Veuillez utiliser un exploit différent."
		),
	},

	[StrId_MissingTargetTitle] =
	{
		STR_EN(
			"The application you attempted to run requires\n"
			"a title that is not installed in the system."
		),
		STR_ES(
			"La aplicación seleccionada necesita un título\n"
			"que no está instalado en el sistema."
		),
		STR_DE(
		        "Die ausgewählte Anwendung benötigt einen\n"
			"Titel der nicht installiert ist"
		),
		STR_FR(
			"L'application sélectionnée requiert un titre\n"
			"qui n'a pas été installé sur le système."
		),
	},

	[StrId_NetLoader] =
	{
		STR_EN("3dslink NetLoader"),
		STR_ES("Cargador de programas 3dslink"),
		STR_DE("3dslink Netzwerk-Loader"),
		STR_FR("Chargeur de programme 3dslink"),
	},

	[StrId_NetLoaderUnavailable] =
	{
		STR_EN("The NetLoader is currently unavailable."),
		STR_ES("El cargador de programas no está disponible."),
		STR_DE("Der Netzwerk-Loader ist zur Zeit nicht verfügbar."),
		STR_FR("Le chargeur de programme 3dslink est indisponible."),
	},

	[StrId_NetLoaderError] =
	{
		STR_EN("An error occurred.\nTechnical details: [%s:%d]"),
		STR_ES("Ha ocurrido un error.\nDatos técnicos: [%s:%d]"),
		STR_DE("Ein Fehler ist aufgetreten\nTechnische Details: [%s:%d]"),
		STR_FR("Une erreur s'est produite.\nDétails techniques : [%s:%d]"),
	},

	[StrId_NetLoaderActive] =
	{
		STR_EN(
			"Waiting for 3dslink to connect…\n"
			"IP Addr: %lu.%lu.%lu.%lu, Port: %d\n\n"
			"  \xEE\x80\x81 Cancel"
		),
		STR_ES(
			"Esperando a que se conecte 3dslink…\n"
			"Dir.IP: %lu.%lu.%lu.%lu, Puerto: %d\n\n"
			"  \xEE\x80\x81 Cancelar"
		),
		STR_DE(
			"Warte auf Verbindung von 3dslink…\n"
			"IP Addr: %lu.%lu.%lu.%lu, Port: %d\n\n"
			"  \xEE\x80\x81 Abbrechen"
		),
		STR_FR(
			"En attente de la connexion de 3dslink…\n"
			"Adr. IP : %lu.%lu.%lu.%lu, Port : %d\n\n"
			"  \xEE\x80\x81 Annuler"
		),
	},

	[StrId_NetLoaderTransferring] =
	{
		STR_EN(
			"Transferring…\n"
			"%zu out of %zu KiB written"
		),
		STR_ES(
			"Transfiriendo…\n"
			"%zu de %zu KiB escritos"
		),
		STR_ES(
			"Übertragen…\n"
			"%zu von %zu KiB geschrieben"
		),
		STR_FR(
			"Transfert…\n"
			"%zu sur %zu KiB écrits"
		),
	},
};
