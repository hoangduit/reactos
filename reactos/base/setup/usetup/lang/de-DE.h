#pragma once

MUI_LAYOUTS deDELayouts[] =
{
    { L"0407", L"00000407" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY deDELanguagePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Sprachauswahl",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Bitte w�hlen Sie die Sprache, die Sie w�hrend der Installation",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        11,
        "verwenden wollen.  Best�tigen Sie die Auswahl mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Diese Sprache wird sp�ter als Standardsprache im System verwendet.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortsetzen  F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEWelcomePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Willkommen zum ReactOS-Setup",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Dieser Teil des Setups kopiert das ReactOS-Betriebssystem auf Ihren",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Computer und bereitet die n�chsten Schritte vor.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Dr�cken Sie die EINGABETASTE, um ReactOS zu installieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Dr�cken Sie R, um ReactOS zu reparieren oder aktualisieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Dr�cken Sie L, um die Lizenzabkommen von ReactOS zu lesen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Dr�cken Sie F3, um die Installation abzubrechen.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Weitere Informationen erhalten Sie unter:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "http://www.reactos.org",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "EINGABETASTE = Fortsetzen  R = Reparieren  L = Lizenz  F3 = Beenden",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEIntroPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Der Installationsassistent befindet sich noch in der Entwicklungsphase.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Einige Funktionen werden noch nicht vollst�ndig unterst�tzt.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Es existieren folgende Beschr�nkungen:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Nur prim�re Festplattenpartitionen k�nnen verwaltet werden.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Eine prim�re Partition kann nicht gel�scht werden,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  solange erweiterte Partitionen auf der Festplatte existieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Die jeweils erste erweiterte Partition kann nicht gel�scht werden,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  solange weitere erweiterte Partitionen auf der Festplatte existieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Es werden nur FAT-Dateisysteme unterst�tzt.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- Die Dateisystem�berpr�fung ist noch nicht implementiert.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  Dr�cken Sie die EINGABETASTE, um ReactOS zu installieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  Dr�cken Sie F3, um die Installation abzubrechen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortsetzen   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS| TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDELicensePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "Lizenz:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "ReactOS ist unter den Bedingungen der GNU General Public License",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "lizenziert. Einige Teile von ReactOS stehen unter dazu kompatiblen",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "Lizenzen wie der BSD- oder GNU LGPL-Lizenz.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "S�mtliche Software in ReactOS daher unter der",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "GNU GPL ver�ffentlicht, behalten daneben aber ihre",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "urspr�nglichen Lizenzen bei.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "ReactOS ist freie Software. Die Ver�ffentlichung dieses Programms",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "erfolgt in der Hoffnung, dass es Ihnen von Nutzen sein wird,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "aber OHNE IRGENDEINE GARANTIE,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "sogar ohne die implizite Garantie der MARKTREIFE",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "oder der VERWENDBARKEIT F�R EINEN BESTIMMTEN ZWECK.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "Details finden Sie in der GNU General Public License.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        22,
        "Sie sollten ein Exemplar der GNU General Public License",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "zusammen mit ReactOS erhalten haben.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        24,
        "Falls nicht, besuchen Sie bitte",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        26,
        "http://www.gnu.org/licenses",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "EINGABETASTE = Zur�ck",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEDevicePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Die untere Liste zeigt die derzeitigen Ger�teeinstellungen.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "Computertyp:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "Anzeige:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "Tastatur:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "Tastaturlayout:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
  /*{
        24,
        16,
        "Akzeptieren:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },*/
    {
        25,
        16, "Diese Ger�teeinstellungen akzeptieren",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Sie k�nnen die Einstellungen durch die PFEILTASTEN ausw�hlen.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "Dr�cken Sie die EINGABETASTE, um eine Einstellung zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        21,
        " ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Wenn alle Einstellungen korrekt sind, w�hlen Sie \"Diese Ger�te-",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "einstellungen akzeptieren\" und best�tigen mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortsetzen   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDERepairPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Der Installationsassistent ist noch der Entwicklungsphase.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Er unterst�tzt noch nicht alle Funktionen eines vollst�ndig",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "nutzbaren Setups.",
        TEXT_STYLE_NORMAL
    },
	    {
        6,
        14,
        "Die Reparaturfunktionen sind noch nicht implementiert.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "\x07  Dr�cken Sie U, um ReactOS zu aktualisieren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Dr�cken Sie R, f�r die Wiederherstellungskonsole.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        20,
        "\x07  Dr�cken Sie ESC, um zur Hauptseite zur�ckzukehren.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        22,
        "\x07  Dr�cken Sie die EINGABETASTE, um den Computer neu zu starten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "ESC = Zur�ck  U = Aktualisieren  R = Wiederherst.  EINGABETASTE = Neustart",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};
static MUI_ENTRY deDEComputerPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Den zu installierenden Computertyp einstellen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Dr�cken Sie die PFEILTASTEN, um den gew�nschten",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Typ zu w�hlen. Best�tigen Sie mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ohne den Computertyp zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortsetzen   ESC = Zur�ck  F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEFlushPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Die geschrieben Daten werden �berpr�ft",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Dies kann einige Zeit in Anspruch nehmen.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "Der PC wird automatisch neu gestartet, sobald der Vorgang beendet ist.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Der Zwischenspeicher wird geleert",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEQuitPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "ReactOS wurde nicht vollst�ndig installiert",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Entfernen Sie alle Datentr�ger aus den CD-Laufwerken.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Dr�cken Sie die EINGABETASTE, um den Computer neu zu starten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Bitte warten ...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEDisplayPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Sie wollen den zu installierenden Bildschirmtyp �ndern.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
        "\x07  Benutzen Sie die PFEILTASTEN, um den gew�nschten",
         TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Typ zu w�hlen. Best�tigen Sie mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren, ohne",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   den Bildschirmtyp zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EEINGABETASTE = Fortsetzen   ESC = Zur�ck  F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDESuccessPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Die Grundkomponenten von ReactOS wurden erfolgreich installiert.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Entfernen Sie alle Datentr�ger aus den CD-Laufwerken.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Dr�cken Sie die EINGABETASTE, um den Computer neu zu starten.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Computer neu starten",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEBootPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Der Bootsektor konnte nicht auf der",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "Festplatte Ihres Computers installiert werden.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "Bitte legen Sie eine formatierte Diskette in Laufwerk A: ein und",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "dr�cken Sie die EINGABETASTE.",
        TEXT_STYLE_NORMAL,
    },
    {
        0,
        0,
        "EINGABETASTE = Fortsetzen   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY deDESelectPartitionEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Diese Liste zeigt die existierenden Partitionen und ",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "freien Speicherplatz f�r neue Partitionen auf der Festplatte an.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Benutzen Sie die PFEILTASTEN, um eine Partition auszuw�hlen.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Best�tigen Sie Ihre Auswahl mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  P erstellt eine prim�re Partition.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  E erstellt eine erweiterte Partition.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  D l�scht eine vorhandene Partition.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Bitte warten...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEFormatPartitionEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Formatierung der Partition",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        10,
        "Die gew�nschte Partition wird nun formatiert.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        11,
        "Dr�cken Sie die EINGABETASTE, um fortzufahren.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortfahren   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY deDEInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Die Installationsdateien werden auf die ausgew�hlte Partition kopiert.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "W�hlen Sie ein Installationsverzeichnis f�r ReactOS:",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "Geben Sie den Namen des Verzeichnisses an.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "Benutzen Sie die Entf-TASTE, um Zeichen zu l�schen.",
        TEXT_STYLE_NORMAL
    },
	{
        6,
        17,
        "Best�tigen Sie die Eingabe mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortfahren   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEFileCopyEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "Die ben�tigten Dateien werden in das Installationsverzeichnis kopiert.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        13,
        "Dieser Vorgang kann einige Zeit in Anspruch nehmen -",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "Bitte haben Sie einen Moment Geduld.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 Bitte warten...    ",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEBootLoaderEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Bestimmen Sie, wo der Bootloader installiert werden soll:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "Bootloader auf der Festplatte installieren (MBR und VBR)",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "Bootloader auf der Festplatte installieren (nur VBR)",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Bootloader auf einer Diskette installieren",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "Bootloader nicht installieren",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortfahren   F3 = Abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Sie wollen den zu installierenden Tastaturtyp �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Benutzen Sie die PFEILTASTEN, um den gew�nschten Typ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "    zu w�hlen. Best�tigen Sie Ihre Auswahl mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ohne den Tastaturtyp zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortfahren   ESC = Abbrechen   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDELayoutSettingsEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Bitte w�hlen Sie das Standardtastaturlayout aus.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  Benutzen Sie die PFEILTASTEN, um den gew�nschten Typ",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   zu w�hlen. Best�tigen Sie Ihre Auswahl mit der EINGABETASTE.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Dr�cken Sie ESC, um zur vorherigen Seite zur�ckzukehren,",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   ohne das Tastaturlayout zu �ndern.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "EINGABETASTE = Fortfahren   ESC = Abbrechen   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY deDEPrepareCopyEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Der Computer wird f�r die Installation vorbereitet.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Erstelle Liste der zu kopierenden Dateien...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY deDESelectFSEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "W�hlen Sie ein Dateisystem aus der folgenden Liste aus.",
        0
    },
    {
        8,
        19,
        "\x07  Dr�cken Sie die PFEILTASTEN, um das Dateisystem zu �ndern.",
        0
    },
    {
        8,
        21,
        "\x07  Dr�cken Sie die EINGABETASTE, um die Partition zu formatieren.",
        0
    },
    {
        8,
        23,
        "\x07  Dr�cken Sie ESC, um eine andere Partition auszuw�hlen.",
        0
    },
    {
        0,
        0,
        "EINGABETASTE = Fortfahren   ESC = Zur�ck   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDEDeletePartitionEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Sie haben sich entschieden, diese Partition zu l�schen",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  Dr�cken Sie D, um die Partition zu l�schen.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "Warnung: Alle Daten auf dieser Partition werden gel�scht!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ESC um abzubrechen.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = L�sche Partition   ESC = Abbrechen   F3 = Installation abbrechen",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY deDERegistryEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Setup ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Systemkonfiguration wird aktualisiert. ",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        0,
        0,
        "Registrierungseintr�ge erstellen...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR deDEErrorEntries[] =
{
    {
        // NOT_AN_ERROR
        "Erfolg\n"
    },
    {
        //ERROR_NOT_INSTALLED
        "ReactOS wurde nicht vollst�ndig auf Ihrem System installiert.\n"
        "Wenn Sie die Installation jetzt beenden, m�ssen Sie diese\n"
        "erneut starten, um ReactOS zu installieren.\n"
        "\n"
        "  \x07  Dr�cken Sie die EINGABETASTE, um die Installation fortzusetzen.\n"
        "  \x07  Dr�cken Sie F3, um die Installation zu beenden.",
        "F3 = Beenden  EINGABETASTE = Fortsetzen"
    },
    {
        //ERROR_NO_HDD
        "Es konnte keine Festplatte gefunden werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Es konnte kein Installationsmedium gefunden werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "TXTSETUP.SIF konnte nicht gefunden werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "TXTSETUP.SIF scheint besch�digt zu sein.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Es wurde eine ung�ltige Signatur in TXTSETUP.SIF gefunden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Es konnten keine Laufwerksinformationen abgefragt werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_WRITE_BOOT,
        "Der FAT-Bootcode konnte nicht auf der Partition installiert werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Computertypenliste konnte nicht geladen werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Displayeinstellungsliste konnte nicht geladen werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Tastaturtypenliste konnte nicht geladen werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Die Liste der Tastaturlayouts konnte nicht geladen werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_WARN_PARTITION,
        "Es wurde mindestens eine Festplatte mit einer inkompatiblen\n"
        "Partitionstabelle gefunden, die nicht richtig verwendet werden kann.\n"
        "\n"
        "�nderungen an den Partitionen k�nnen die Partitionstabelle zerst�ren!\n"
        "\n"
        "  \x07  Dr�cken Sie F3, um die Installation zu beenden.\n"
        "  \x07  Dr�cken Sie die EINGABETASTE, um die Installation fortzusetzen.",
        "F3 = Beenden  ENTER = EINGABETASTE"
    },
    {
        //ERROR_NEW_PARTITION,
        "Sie k�nnen keine neue Partition in einer bereits\n"
        "vorhandenen Partition erstellen!\n"
        "\n"
        "  * * Eine beliebige Taste zum Fortsetzen dr�cken.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "Sie k�nnen unpartitionierten Speicher nicht l�schen!\n"
        "\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Der FAT-Bootcode konnte nicht auf der Partition installiert werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_NO_FLOPPY,
        "Keine Diskette in Laufwerk A: gefunden.",
        "EINGABETASTE = Fortsetzen"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Das Tastaturlayout konnte nicht aktualisiert werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Die Registrierungseintr�ge der Anzeigeeinstellungen\n"
		"konnten nicht aktualisiert werden.",
        "EINGABETASTER = Computer neu starten"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Es konnte keine Hive-Datei importiert werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_FIND_REGISTRY
        "Die Registrierungsdateien konnten nicht gefunden werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_CREATE_HIVE,
        "Die Zweige in der Registrierung konnten nicht erstellt werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Die Registrierung konnte nicht initialisiert werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Das CAB-Archiv besitzt keine g�ltige INF-Datei.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_CABINET_MISSING,
        "Das CAB-Archiv wurde nicht gefunden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Das CAB-Archiv enth�lt kein Setup-Skript.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_COPY_QUEUE,
        "Die Liste mit den zu kopierenden Dateien\n"
		"konnte nicht gefunden werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_CREATE_DIR,
        "Die Installationspfade konnten nicht erstellt werden.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Der Abschnitt 'Ordner' konnte in\n"
        "TXTSETUP.SIF nicht gefunden werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_CABINET_SECTION,
        "Der Abschnitt 'Ordner' konnte\n"
        "im im CAB-Archiv nicht gefunden werden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Setup konnte den Installationspfad nicht erstellen.",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Setup konnte die 'SetupData'-Sektion in\n"
        "TXTSETUP.SIF nicht finden.\n",
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Die Partitionstabellen konnten nicht geschrieben werden.\n"
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Es konnte kein Codepage-Eintrag hinzugef�gt werden.\n"
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Die Systemsprache konnte nicht eingestellt werden.\n"
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Die Tastaturlayouts konnten nicht in die Registrierung\n"
        "eingetragen werden.\n"
        "EINGABETASTE = Computer neu starten"
    },
        {
        //ERROR_UPDATE_GEOID,
        "Der geografische Standort konnte nicht eingestellt werden.\n"
        "EINGABETASTE = Computer neu starten"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Es ist nicht gen�gend Speicherplatz auf der\n"
        "gew�hlten Partition vorhanden.\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken.",
        NULL
    },
    {
        //ERROR_PARTITION_TABLE_FULL,
        "Sie k�nnen keine weitere prim�re oder erweiterte Partition in\n"
        "der Partitionstabelle erstellen, weil die Tabelle voll ist.\n"
        "\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken."
    },
    {
        //ERROR_ONLY_ONE_EXTENDED,
        "Sie k�nnen nur eine erweiterte Partition auf jeder Festplatte anlegen.\n"
        "\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken."
    },
    {
        //ERROR_NOT_BEHIND_EXTENDED,
        "Sie k�nnen hinter einer erweiterten Partition keine weitere Partition anlegen.\n"
        "\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken."
    },
    {
        //ERROR_EXTENDED_NOT_LAST,
        "Eine erweiterte Partition muss immer die letzte Partition in \n"
        "einer Partitionstabelle sein.\n"
        "\n"
        "  * Eine beliebige Taste zum Fortsetzen dr�cken."
    },
    {
        NULL,
        NULL
    }
};


MUI_PAGE deDEPages[] =
{
    {
        LANGUAGE_PAGE,
        deDELanguagePageEntries
    },
    {
        START_PAGE,
        deDEWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        deDEIntroPageEntries
    },
    {
        LICENSE_PAGE,
        deDELicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        deDEDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        deDERepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        deDEComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        deDEDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        deDEFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        deDESelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        deDESelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        deDEFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        deDEDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        deDEInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        deDEPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        deDEFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        deDEKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        deDEBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        deDELayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        deDEQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        deDESuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        deDEBootPageEntries
    },
    {
        REGISTRY_PAGE,
        deDERegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING deDEStrings[] =
{
    {STRING_PLEASEWAIT,
     "   Bitte warten..."},
    {STRING_INSTALLCREATEPARTITION,
     "  EINGABETASTE = Installieren  P = Prim�re  E = Erweiterte  F3 = Installation abbr."},
    {STRING_INSTALLDELETEPARTITION,
     "  EINGABETASTE = Installieren  D = Partition l�schen  F3 = Installation abbr."},
    {STRING_DELETEPARTITION,
     "   D = Partition l�schen  F3 = Installation abbrechen"},
    {STRING_PARTITIONSIZE,
     "Gr��e der neuen Partition:"},
    {STRING_CHOOSENEWPARTITION,
     "Eine prim�re Partition soll hier erstellt werden:"},
    {STRING_CHOOSE_NEW_EXTENDED_PARTITION,
     "Eine erweiterte Partition soll hier erstellt werden:"},
    {STRING_HDDSIZE,
    "Bitte geben Sie die Gr��e der neuen Partition in Megabyte ein."},
    {STRING_CREATEPARTITION,
     "  EINGABETASTE = Partition erstellen  ESC = Abbrechen  F3 = Installation abbr."},
    {STRING_PARTFORMAT,
    "Diese Partition wird als n�chstes formatiert."},
    {STRING_NONFORMATTEDPART,
    "Sie wollen ReactOS auf einer neuen/unformatierten Partition installieren."},
    {STRING_INSTALLONPART,
    "ReactOS wird auf dieser Partition installiert."},
    {STRING_CHECKINGPART,
    "Die ausgew�hlte Partition wird �berpr�ft."},
    {STRING_QUITCONTINUE,
    "F3 = Beenden  EINGABETASTE = Fortsetzen"},
    {STRING_REBOOTCOMPUTER,
    "EINGABETASTE = Computer neu starten"},
    {STRING_TXTSETUPFAILED,
    "Der Abschnitt '%S'in TXTSETUP.SIF\nkonnte nicht gefunden werden.\n"},
    {STRING_COPYING,
     "   Kopiere Datei: %S"},
    {STRING_SETUPCOPYINGFILES,
     "Dateien werden kopiert..."},
    {STRING_REGHIVEUPDATE,
    "   Registrierungseintr�ge werden aktualisiert..."},
    {STRING_IMPORTFILE,
    "   Importiere %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   Anzeigeeinstellungen werden aktualisiert..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Lokalisierungseinstellungen werden aktualisiert..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   Tastaturlayouteinstellungen werden aktualisiert..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   Codepage-Informationen werden hinzugef�gt..."},
    {STRING_DONE,
    "   Fertig..."},
    {STRING_REBOOTCOMPUTER2,
    "   EINGABETASTE = Computer neu starten"},
    {STRING_CONSOLEFAIL1,
    "Konsole konnte nicht ge�ffnet werden\r\n\r\n"},
    {STRING_CONSOLEFAIL2,
    "Der h�ufigste Grund hierf�r ist die Verwendung einer USB-Tastatur\r\n"},
    {STRING_CONSOLEFAIL3,
    "USB-Tastaturen werden noch nicht vollst�ndig unterst�tzt\r\n"},
    {STRING_FORMATTINGDISK,
    "Ihre Festplatte wird formatiert"},
    {STRING_CHECKINGDISK,
    "Ihre Festplatte wird �berpr�ft"},
    {STRING_FORMATDISK1,
    " Partition mit dem %S-Dateisystem formatieren (Schnell) "},
    {STRING_FORMATDISK2,
    " Partition mit dem %S-Dateisystem formatieren "},
    {STRING_KEEPFORMAT,
    " Dateisystem beibehalten (Keine Ver�nderungen) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu) auf %wZ."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "auf %I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu) auf %wZ."},
    {STRING_HDDINFOUNK3,
    "auf %I64u %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "Festplatte %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  Typ %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "auf Festplatte %lu (%I64u %s), Port=%hu, Bus=%hu, Id=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  %sTyp %-3u%s                       %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu) auf %S"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  Festplatte %lu  (Port=%hu, Bus=%hu, Id=%hu)"},
    {STRING_NEWPARTITION,
    "Setup erstellte eine neue Partition auf"},
    {STRING_UNPSPACE,
    "    %sUnpartitionierter Speicher%s     %6lu %s"},
    {STRING_MAXSIZE,
    "MB (max. %lu MB)"},
    {STRING_EXTENDED_PARTITION,
    "Erweiterte Partition"},
    {STRING_UNFORMATTED,
    "Neu (Unformatiert)"},
    {STRING_FORMATUNUSED,
    "Ungenutzt"},
    {STRING_FORMATUNKNOWN,
    "Unbekannt"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "Tastaturlayout hinzuf�gen"},
    {0, 0}
};
