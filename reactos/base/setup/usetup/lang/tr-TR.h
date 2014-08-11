/* TRANSLATOR: 2013 Erdem Ersoy (eersoy93) (erdemersoy@live.com) */

#pragma once

MUI_LAYOUTS trTRLayouts[] =
{
    { L"041F", L"0000041F" },
    { L"041F", L"0001041f" },
    { L"0409", L"00000409" },
    { NULL, NULL }
};

static MUI_ENTRY trTRLanguagePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Dil Se�imi",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  L�tfen kurulum s�reci i�in kullan�lacak dili se�iniz.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Ard�ndan Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Bu dil, kurulacak dizgenin �n tan�ml� dili olacakt�r.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRWelcomePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "ReactOS Kur'a ho� geldiniz.",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        6,
        11,
        "Kurulumun bu b�l�m�, ReactOS ��letim Dizgesi'ni bilgisayar�n�za",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "�o�alt�r ve kurulumun ikinci b�l�m�n� an�klar.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ReactOS'u kurmak i�in Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  ReactOS'u onarmak vey� y�kseltmek i�in R'ye bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  ReactOS Ruhsatlama Ist�lahlar� ve Ko�ullar�'n� g�r�nt�lemek i�in L'ye bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  ReactOS'u kurmadan ��kmak i�in F3'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "Daha �ok bilgi i�in l�tfen u�ray�n�z:",
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
        "Giri� = S�rd�r  R = Onar vey� Y�kselt  L = Ruhsat F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRIntroPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "ReactOS Kur, bir �n geli�me evresindedir. Daha t�m�yle kullan��l�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "bir kurulum uygulamas�n�n t�m i�levlerini desteklemez.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "A�a��daki k�s�tlamalar uygulan�r:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "- Kur, bir diskte birden �ok ana b�l�m� y�netemez.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "- Kur, bir diskte geni�letilmi� b�l�mler oldu�u s�rece",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "  o diskten bir ana b�l�m silemez.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        "- Kur, bir diskte ba�ka geni�letilmi� b�l�mlerin oldu�u s�rece",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "  o diskten ilk geni�letilmi� b�l�m� silemez.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "- Kur, yaln�zca FAT k�t�k dizgelerini destekler.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "- K�t�k dizgesi denetimi daha bitirilmemi�tir.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        23,
        "\x07  ReactOS'u kurmak i�in Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "\x07  ReactOS'u kurmadan ��kmak i�in F3'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRLicensePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        6,
        "Ruhsatlama:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        8,
        "ReactOS Dizgesi, GNU GPL'yle X11, BSD ve GNU LPGL",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        9,
        "ruhsatlar� gibi ba�ka uygun ruhsatlardan kod i�eren",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "k�s�mlar�n ko�ullar� alt�nda ruhsatlanm��t�r.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "Bu y�zden ReactOS dizgesinin k�sm� olan t�m yaz�l�mlar, korunan",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "�zg�n ruhsat�yla birlikte GNU GPL alt�nda yay�nlan�r.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "Bu yaz�l�m, yerli ve uluslararas� yasa uygulanabilir kullan�m",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�zerine hi�bir g�vence ve k�s�tlamayla gelmez. ReactOS'un",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        16,
        " ruhsatlanmas� yaln�zca ���nc� yanlara da��tmay� kapsar.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "E�er birtak�m nedenlerden dolay� ReactOS ile GNU Um�m�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "Kamu Ruhsat�'n�n bir kopyas�n� almad�ysan�z l�tfen u�ray�n�z:",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "http://www.gnu.org/licenses/licenses.html",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        21,
        "G�vence:",
        TEXT_STYLE_HIGHLIGHT
    },
    {
        8,
        23,
        "Bu �zg�r yaz�l�md�r, �o�altma ko�ullar� i�in kayna�a bak�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        24,
        "Burada hi�bir g�vence YOKTUR, SATILAB�L�RL�K vey�",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        25,
        "BEL�RL� B�R AMACA UYGUNLUK i�in bile.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = Geri D�n",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRDevicePageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A�a��daki dizelge �imdiki ayg�t ayarlar�n� g�sterir.",
        TEXT_STYLE_NORMAL
    },
    {
        24,
        11,
        "Bilgisayar:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        12,
        "G�r�nt�:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        13,
        "D��me Tak�m�:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        14,
        "D��me Tak�m� D�zeni:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        24,
        16,
        "Do�rula:",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_RIGHT
    },
    {
        25,
        16,
		"Bu ayg�t ayarlar�n� do�rula.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        19,
        "Ayg�t ayarlar�n�, bir se�enek se�mek i�in Yukar� vey� A�a�� d��melerine",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        20,
        "basarak de�i�tirebilirsiniz. Sonra ba�ka ayarlar se�mek i�in Giri�",
        TEXT_STYLE_NORMAL
    },
	{
        6,
        21,
        "d��mesine bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        23,
        "T�m ayarlar uygun oldu�unda ""Bu ayg�t ayarlar�n� do�rula.""y�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        24,
        "se�iniz ve Giri� d��mesine bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRRepairPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "ReactOS Kur, bir �n geli�me evresindedir. Daha t�m�yle kullan��l�",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "bir kurulum uygulamas�n�n t�m i�levlerini desteklemez.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "Onarma i�levleri daha bitirilmemi�tir.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  ��letim dizgesini y�kseltmek i�in U'ya bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Kurtarma Konsolu i�in R'ye bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Ana sayfaya geri d�nmek i�in ��k��'a bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  Bilgisayar�n�z� yeniden ba�latmak i�in Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "��k�� = Ana Sayfa  U = Y�kselt  R = Kurtarma  Giri� = Yeniden Ba�lat",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRComputerPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kurulum yap�lacak bilgisayar�n t�r�n� se�mek isteyebilirsiniz.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  �stenen bilgisyar t�r�n� se�mek i�in Yukar�'ya vey� A�a��'ya bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Ard�ndan Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Bilgisayar t�r�n� de�i�tirmeden bir �nceki sayfaya",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   d�nmek i�in ��k�� d��mesine bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   ��k�� = �ptal   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRFlushPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "Dizge, �imdi diskinize saklanm�� t�m veriyi do�ruluyor.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "Bu bir dak�ka s�rebilir.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "Bitti�inde bilgisayar�n�z kendili�inden yeniden ba�layacakt�r.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "�n bellek ar�n�yor...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRQuitPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "ReactOS, t�m�yle kurulmad�.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "A: s�r�c�s�nden disketi ve t�m CD s�r�c�lerinden",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "CD-ROM'lar� ��kar�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Bilgisayar�n�z� yeniden ba�latmak i�in Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "L�tfen bekleyiniz...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG,
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRDisplayPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kurulum yap�lacak g�r�nt�n�n t�r�n� se�mek isteyebilirsiniz.",
        TEXT_STYLE_NORMAL
    },
    {   8,
        10,
        "\x07  �stenen g�r�nt� t�r�n� se�mek i�in Yukar�'ya vey� A�a��'ya bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Ard�ndan Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  G�r�nt� t�r�n� de�i�tirmeden bir �nceki sayfaya",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   d�nmek i�in ��k�� d��mesine bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   ��k�� = �ptal   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRSuccessPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        10,
        6,
        "ReactOS'un ana bile�enleri ba�ar�l� olarak kuruldu.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        8,
        "A: s�r�c�s�nden disketi ve t�m CD s�r�c�lerinden",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        9,
        "CD-ROM'lar� ��kar�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        10,
        11,
        "Bilgisayar�n�z� yeniden ba�latmak i�in Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = Bilgisayar� Yeniden Ba�lat",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRBootPageEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kur, bilgisayar�n�z�n s�bit diskine �n y�kleyiciyi kuramad�.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        12,
        "L�tfen A: s�r�c�s�ne bi�imlendirilmi� bir disket tak�n�z",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        13,
        "ve Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }

};

static MUI_ENTRY trTRSelectPartitionEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "A�a��daki dizelge, var olan b�l�mleri ve yeni b�l�mler i�in",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "kullan�lmayan disk bo�lu�unu g�sterir.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "\x07  Bir dizelge girdisini se�mek i�in Yukar�'ya vey� A�a��'ya bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  Se�ili b�l�me ReactOS'u y�klemek i�in Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "\x07  Bir ana b�l�m olu�turmak i�in P'ye bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        17,
        "\x07  Bir geni�letilmi� b�l�m olu�turmak i�in E'ye bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        19,
        "\x07  Var olan bir b�l�m silmek i�in D'ye bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "L�tfen bekleyiniz...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRFormatPartitionEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "B�l�m Bi�imlendirme",
        TEXT_STYLE_NORMAL
    },
	{
        6,
        10,
        "Kur, �imdi b�l�m� bi�imlendirecek. S�rd�rmek i�in Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        TEXT_STYLE_NORMAL
    }
};

static MUI_ENTRY trTRInstallDirectoryEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kur, se�ili b�l�me ReactOS k�t�klerini y�kler. ReactOS'un",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        9,
        "y�klenmesini istedi�iniz bir dizin se�iniz.",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        14,
        "�nerilen dizini de�i�tirmek i�in, damgalar� silmek i�in Silme'ye bas�n�z",
        TEXT_STYLE_NORMAL
    },
    {
        6,
        15,
        "ve ard�ndan ReactOS'un y�klenmesini istedi�iniz dizini yaz�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRFileCopyEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        0,
        12,
        "ReactOS Kur, ReactOS kurulum dizininize k�t�kleri �o�alt�rken",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
	{
        0,
        13,
        " l�tfen bekleyiniz.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        0,
        14,
        "Bu, bitirmek i�in birka� dak�ka s�rebilir.",
        TEXT_STYLE_NORMAL | TEXT_ALIGN_CENTER
    },
    {
        50,
        0,
        "\xB3 L�tfen bekleyiniz...",
        TEXT_TYPE_STATUS
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRBootLoaderEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kur, �n y�kleyiciyi kuruyor.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        12,
        "�n y�kleyiciyi s�bit diskin �zerine kur. (MBR ve VBR)",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "�n y�kleyiciyi s�bit diskin �zerine kur. (Yaln�zca VBR)",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "�n y�kleyiciyi bir diskete kur.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        15,
        "�n y�kleyici kurulumunu ge�.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRKeyboardSettingsEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kurulum yap�lacak d��me tak�m�n�n t�r�n� se�mek isteyebilirsiniz.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  �stenen d��me tak�m� t�r�n� se�mek i�in Yukar�'ya vey� A�a��'ya bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Ard�ndan Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  D��me tak�m� t�r�n� de�i�tirmeden bir �nceki sayfaya",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   d�nmek i�in ��k�� d��mesine bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   ��k�� = �ptal   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRLayoutSettingsEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "L�tfen �n tan�ml� olarak kurulacak bir d�zen se�iniz.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        10,
        "\x07  �stenen d��me tak�m� d�zenini se�mek i�in Yukar�'ya vey� A�a��'ya bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        11,
        "   Ard�ndan Giri�'e bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        13,
        "\x07  G�r�nt� t�r�n� de�i�tirmeden bir �nceki sayfaya",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        14,
        "   d�nmek i�in ��k�� d��mesine bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "Giri� = S�rd�r   ��k�� = �ptal   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY trTRPrepareCopyEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kur, ReactOS k�t�klerini �o�altmak i�in bilgisayar�n�z� an�kl�yor.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "K�t�k �o�altma dizelgesi olu�turuluyor...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

static MUI_ENTRY trTRSelectFSEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        17,
        "A�a��daki dizelgeden bir k�t�k dizgesi se�iniz.",
        0
    },
    {
        8,
        19,
        "\x07  Bir k�t�k dizgesi se�mek i�in Yukar�'ya vey� A�a��'ya bas�n�z.",
        0
    },
    {
        8,
        21,
        "\x07  B�l�m� bi�imlendirmek i�in Giri�'e bas�n�z.",
        0
    },
    {
        8,
        23,
        "\x07  Ba�ka bir b�l�m se�mek i�in ��k��'a bas�n�z.",
        0
    },
    {
        0,
        0,
        "Giri� = S�rd�r   ��k�� = �ptal   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },

    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRDeletePartitionEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "B�l�m� silmeyi se�tiniz.",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        18,
        "\x07  B�l�m� silmek i�in D'ye bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        11,
        19,
        "UYARI: Bu b�l�mdeki t�m veriler yitirilecektir!",
        TEXT_STYLE_NORMAL
    },
    {
        8,
        21,
        "\x07  �ptal etmek i�in ��k��'a bas�n�z.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "D = B�l�m Sil   ��k�� = �ptal   F3 = ��k",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    }
};

static MUI_ENTRY trTRRegistryEntries[] =
{
    {
        4,
        3,
        " ReactOS " KERNEL_VERSION_STR " Kur ",
        TEXT_STYLE_UNDERLINE
    },
    {
        6,
        8,
        "Kur, dizge yap�land�rmas�n� �imdikile�tiriyor.",
        TEXT_STYLE_NORMAL
    },
    {
        0,
        0,
        "De�er y���nlar� olu�turuluyor...",
        TEXT_TYPE_STATUS | TEXT_PADDING_BIG
    },
    {
        0,
        0,
        NULL,
        0
    },

};

MUI_ERROR trTRErrorEntries[] =
{
    {
        // NOT_AN_ERROR
        "Ba�ar�l�\n"
    },
    {
        //ERROR_NOT_INSTALLED
        "ReactOS, bilgisayara t�m�yle kurulmad�. E�er �imdi\n"
        "Kur'dan ��karsan�z ReactOS'u kurmak i�in Kur'u\n"
        "yeniden �al��t�rmaya gereksinim duyacaks�n�z.\n"
        "\n"
        "  \x07  Kur'u s�rd�rmek i�in Giri�'e bas�n�z.\n"
        "  \x07  Kur'dan ��kmak i�in F3'e bas�n�z.",
        "F3 = ��k  Giri� = S�rd�r"
    },
    {
        //ERROR_NO_HDD
        "Kur, bir s�bit disk bulamad�.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_NO_SOURCE_DRIVE
        "Kur, kaynak s�r�c�y� bulamad�.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_LOAD_TXTSETUPSIF
        "Kur, TXTSETUP.SIF k�t���n� y�klemede ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_CORRUPT_TXTSETUPSIF
        "Kur, bozuk bir TXTSETUP.SIF buldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_SIGNATURE_TXTSETUPSIF,
        "Kur, TXTSETUP.SIF'ta ge�ersiz bir im buldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_DRIVE_INFORMATION
        "Kur, dizge s�r�c� bilgisini alamad�.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_WRITE_BOOT,
        "Kur, dizge b�l�m�ne FAT �n y�kleme kodunu kuramad�.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_LOAD_COMPUTER,
        "Kur, bilgisayar t�r� dizelgesini y�klemede ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_LOAD_DISPLAY,
        "Kur, g�r�nt� ayarlar� dizelgesini y�klemede ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_LOAD_KEYBOARD,
        "Kur, d��me tak�m� t�r� dizelgesini y�klemede ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_LOAD_KBLAYOUT,
        "Kur, d��me tak�m� d�zeni dizelgesini y�klemede ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_WARN_PARTITION,
        "Kur, d�zg�n y�netilemeyen bir uyumsuz b�l�m tablosu i�eren en az\n"
		"bir s�bit disk buldu!\n"
        "\n"         
        "B�l�mleri olu�turmak vey� silmek b�l�m tablosunu yok edebilir.\n"
        "\n"
        "  \x07  Kur'dan ��kmak i�in F3'e bas�n�z.\n"
        "  \x07  S�rd�rmek i�in Giri�'e bas�n�z.",
        "F3 = ��k   Giri� = S�rd�r"
    },
    {
        //ERROR_NEW_PARTITION,
        "�neden var olan bir b�l�m�n i�ine yeni\n"
        "bir b�l�m olu�turamazs�n�z!\n"
        "\n"
        "  * S�rd�rmek i�in bir d��meye bas�n�z.",
        NULL
    },
    {
        //ERROR_DELETE_SPACE,
        "B�l�mlenmemi� disk bo�lu�unu silemezsiniz!\n"
        "\n"
        "  * S�rd�rmek i�in bir d��meye bas�n�z.",
        NULL
    },
    {
        //ERROR_INSTALL_BOOTCODE,
        "Kur, dizge b�l�m� �zerinde FAT �n y�kleme kodunu kurmada ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_NO_FLOPPY,
        "A: s�r�c�s�nde disk yok.",
        "Giri� = S�rd�r"
    },
    {
        //ERROR_UPDATE_KBSETTINGS,
        "Kur, d��me tak�m� d�zeni ayarlar�n� �imdikile�tirmede ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_UPDATE_DISPLAY_SETTINGS,
        "Kur, g�r�nt� de�er ayarlar�n� �imdikile�tirmede ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_IMPORT_HIVE,
        "Kur, bir y���n k�t��� almada ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_FIND_REGISTRY
        "Kur, de�er veri k�t�klerini bulmada ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_CREATE_HIVE,
        "Kur, de�er y���nlar�n� olu�turmada ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_INITIALIZE_REGISTRY,
        "Kur, De�er Defteri'ni ba�latmada ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_INVALID_CABINET_INF,
        "Dolab�n ge�erli yap�land�rma k�t��� yok.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_CABINET_MISSING,
        "Dolap bulunamad�.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_CABINET_SCRIPT,
        "Dolab�n kurulum beti�i yok.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_COPY_QUEUE,
        "Kur, k�t�k �o�altma kuyru�unu a�mada ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_CREATE_DIR,
        "Kur, kurulum dizinlerini olu�turmada ba�ar�s�z oldu.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_TXTSETUP_SECTION,
        "Kur, TXTSETUP.SIF'de ""Directories"" b�l�m�n�\n"
        "bulmada ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_CABINET_SECTION,
        "Kur, dolapta ""Directories"" b�l�m�n�\n"
        "bulmada ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_CREATE_INSTALL_DIR
        "Kur, kurulum dizinini olu�turamad�.",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_FIND_SETUPDATA,
        "Kur, TXTSETUP.SIF'de ""SetupData"" b�l�m�n�\n"
        "bulmada ba�ar�s�z oldu.\n",
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_WRITE_PTABLE,
        "Kur, b�l�m tablolar� yazmada ba�ar�s�z oldu.\n"
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_ADDING_CODEPAGE,
        "Kur, De�er Defteri'ne kod sayfas� eklemede ba�ar�s�z oldu.\n"
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_UPDATE_LOCALESETTINGS,
        "Kur, dizge yerli ay�r�n� yapamad�.\n"
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_ADDING_KBLAYOUTS,
        "Kur, De�er Defteri'ne d��me tak�m� d�zenleri eklemede ba�ar�s�z oldu.\n"
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_UPDATE_GEOID,
        "Kur, co�r�f� kimli�i ayarlayamad�.\n"
        "Giri� = Bilgisayar� Yeniden Ba�lat"
    },
    {
        //ERROR_INSUFFICIENT_DISKSPACE,
        "Se�ili b�l�mde yeterli bo� alan yok.\n"
        "  * S�rd�rmek i�in bir d��meye bas�n�z.",
        NULL
    },
    {
        //ERROR_PARTITION_TABLE_FULL,
        "B�l�m tablosu dolu oldu�undan dolay� bu diskin b�l�m tablosunda\n"
        "yeni bir ana b�l�m ya da geni�letilmi� b�l�m olu�turamazs�n�z.\n"
        "\n"
        "  * S�rd�rmek i�in bir d��meye bas�n�z."
    },
    {
        //ERROR_ONLY_ONE_EXTENDED,
        "Bir diskte birden �ok geni�letilmi� b�l�m olu�turamazs�n�z.\n"
        "\n"
        "  * S�rd�rmek i�in bir d��meye bas�n�z."
    },
    {
        //ERROR_NOT_BEHIND_EXTENDED,
        "Bir geni�letilmi� b�l�m arkas�nda bir b�l�m olu�turamazs�n�z.\n"
        "\n"
        "  * S�rd�rmek i�in bir d��meye bas�n�z."
    },
    {
        //ERROR_EXTENDED_NOT_LAST,
        "Bir geni�letilmi� b�l�m, her zaman bir b�l�m\n"
        "tablosunda son b�l�m olmal�d�r.\n"
        "\n"
        "  * S�rd�rmek i�in bir d��meye bas�n�z."
    },
    {
        NULL,
        NULL
    }
};

MUI_PAGE trTRPages[] =
{
    {
        LANGUAGE_PAGE,
        trTRLanguagePageEntries
    },
    {
        START_PAGE,
        trTRWelcomePageEntries
    },
    {
        INSTALL_INTRO_PAGE,
        trTRIntroPageEntries
    },
    {
        LICENSE_PAGE,
        trTRLicensePageEntries
    },
    {
        DEVICE_SETTINGS_PAGE,
        trTRDevicePageEntries
    },
    {
        REPAIR_INTRO_PAGE,
        trTRRepairPageEntries
    },
    {
        COMPUTER_SETTINGS_PAGE,
        trTRComputerPageEntries
    },
    {
        DISPLAY_SETTINGS_PAGE,
        trTRDisplayPageEntries
    },
    {
        FLUSH_PAGE,
        trTRFlushPageEntries
    },
    {
        SELECT_PARTITION_PAGE,
        trTRSelectPartitionEntries
    },
    {
        SELECT_FILE_SYSTEM_PAGE,
        trTRSelectFSEntries
    },
    {
        FORMAT_PARTITION_PAGE,
        trTRFormatPartitionEntries
    },
    {
        DELETE_PARTITION_PAGE,
        trTRDeletePartitionEntries
    },
    {
        INSTALL_DIRECTORY_PAGE,
        trTRInstallDirectoryEntries
    },
    {
        PREPARE_COPY_PAGE,
        trTRPrepareCopyEntries
    },
    {
        FILE_COPY_PAGE,
        trTRFileCopyEntries
    },
    {
        KEYBOARD_SETTINGS_PAGE,
        trTRKeyboardSettingsEntries
    },
    {
        BOOT_LOADER_PAGE,
        trTRBootLoaderEntries
    },
    {
        LAYOUT_SETTINGS_PAGE,
        trTRLayoutSettingsEntries
    },
    {
        QUIT_PAGE,
        trTRQuitPageEntries
    },
    {
        SUCCESS_PAGE,
        trTRSuccessPageEntries
    },
    {
        BOOT_LOADER_FLOPPY_PAGE,
        trTRBootPageEntries
    },
    {
        REGISTRY_PAGE,
        trTRRegistryEntries
    },
    {
        -1,
        NULL
    }
};

MUI_STRING trTRStrings[] =
{
    {STRING_PLEASEWAIT,
    "   L�tfen bekleyiniz..."},
    {STRING_INSTALLCREATEPARTITION,
    "   Giri� = Kur  P = Ana B�l�m Olu�tur  E = Geni�letilmi� B�l�m Olu�tur  F3 = ��k"},
    {STRING_INSTALLCREATELOGICAL,
    "   ENTER = Kur   L = Mant�kl�k B�l�m Olu�tur   F3 = ��k"},
    {STRING_INSTALLDELETEPARTITION,
    "   Giri� = Kur   D = B�l�m� Sil   F3 = ��k"},
    {STRING_DELETEPARTITION,
    "   D = B�l�m� Sil   F3 = ��k"},
    {STRING_PARTITIONSIZE,
    "Yeni b�l�m�n b�y�kl���n� giriniz:"},
    {STRING_CHOOSENEWPARTITION,
    "�zerinde bir ana b�l�m olu�turmay� se�tiniz:"},
    {STRING_CHOOSE_NEW_EXTENDED_PARTITION,
    "�zerinde bir geni�letilmi� b�l�m olu�turmay� se�tiniz:"},
    {STRING_CHOOSE_NEW_LOGICAL_PARTITION,
    "�zerinde bir mant�kl�k b�l�m olu�turmay� se�tiniz:"},
    {STRING_HDDSIZE,
    "L�tfen yeni b�l�m�n b�y�kl���n� megabayt olarak giriniz."},
    {STRING_CREATEPARTITION,
    "   Giri� = B�l�m Olu�tur   ��k�� = �ptal   F3 = ��k"},
    {STRING_PARTFORMAT,
    "Bu b�l�m ileride bi�imlendirilecektir."},
    {STRING_NONFORMATTEDPART,
    "ReactOS'u yeni ya da bi�imlendirilmemi� bir b�l�me kurmay� se�tiniz."},
    {STRING_INSTALLONPART,
    "Kur, ReactOS'u b�l�m �zerine kurar."},
    {STRING_CHECKINGPART,
    "Kur, �imdi se�ili b�l�m� g�zden ge�iriyor."},
    {STRING_QUITCONTINUE,
    "F3 = ��k   Giri� = S�rd�r"},
    {STRING_REBOOTCOMPUTER,
    "Giri� = Bilgisayar� Yeniden Ba�lat"},
    {STRING_TXTSETUPFAILED,
    "Kur, TXTSETUP.SIF'de ""%S"" b�l�m�n�\nbulmada ba�ar�s�z oldu.\n"},
    {STRING_COPYING,
    "   K�t�k �o�alt�l�yor: %S..."},
    {STRING_SETUPCOPYINGFILES,
    "Kur, k�t�kleri �o�alt�yor..."},
    {STRING_REGHIVEUPDATE,
    "   De�er y���nlar� �imdikile�tiriliyor..."},
    {STRING_IMPORTFILE,
    "   Al�n�yor: %S..."},
    {STRING_DISPLAYETTINGSUPDATE,
    "   G�r�nt� ayarlar� de�erleri �imdikile�tiriliyor..."},
    {STRING_LOCALESETTINGSUPDATE,
    "   Yerli ayarlar �imdikile�tiriliyor..."},
    {STRING_KEYBOARDSETTINGSUPDATE,
    "   D��me tak�m� d�zeni ayarlar� �imdikile�tiriliyor..."},
    {STRING_CODEPAGEINFOUPDATE,
    "   De�er Defteri'ne kod sayfas� bilgisi ekleniyor..."},
    {STRING_DONE,
    "   Bitti..."},
    {STRING_REBOOTCOMPUTER2,
    "   Giri� = Bilgisayar� Yeniden Ba�lat"},
    {STRING_CONSOLEFAIL1,
    "Konsol a��lam�yor.\r\n\r\n"},
    {STRING_CONSOLEFAIL2,
    "Bunun en bilinen nedeni, bir USB d��me tak�m� kullan�lmas�d�r.\r\n"},
    {STRING_CONSOLEFAIL3,
    "USB d��me tak�mlar� daha t�m�yle desteklenmemektedir.\r\n"},
    {STRING_FORMATTINGDISK,
    "Kur, diskinizi bi�imlendiriyor."},
    {STRING_CHECKINGDISK,
    "Kur, diskinizi g�zden ge�iriyor."},
    {STRING_FORMATDISK1,
    " B�l�m� %S k�t�k dizgesiyle h�zl� bi�imlendir. "},
    {STRING_FORMATDISK2,
    " B�l�m� %S k�t�k dizgesiyle bi�imlendir. "},
    {STRING_KEEPFORMAT,
    " �imdiki k�t�k dizgesini koru. (De�i�iklik yok.) "},
    {STRING_HDINFOPARTCREATE,
    "%I64u %s  S�bit Disk %lu  (Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu), %wZ �zerinde."},
    {STRING_HDDINFOUNK1,
    "%I64u %s  S�bit Disk %lu  (Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu)."},
    {STRING_HDDINFOUNK2,
    "   %c%c  T�r  %lu    %I64u %s"},
    {STRING_HDINFOPARTDELETE,
    "�zerinde: %I64u %s  S�bit Disk %lu  (Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu), %wZ �zerinde."},
    {STRING_HDDINFOUNK3,
    "�zerinde: %I64u %s  S�bit Disk %lu  (Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu)."},
    {STRING_HDINFOPARTZEROED,
    "S�bit Disk %lu (%I64u %s), Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu (%wZ)."},
    {STRING_HDDINFOUNK4,
    "%c%c  T�r  %lu    %I64u %s"},
    {STRING_HDINFOPARTEXISTS,
    "�zerinde: S�bit Disk %lu (%I64u %s), Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu (%wZ)."},
    {STRING_HDDINFOUNK5,
    "%c%c  %sT�r  %-3u%s                       %6lu %s"},
    {STRING_HDINFOPARTSELECT,
    "%6lu %s  S�bit Disk %lu  (Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu), %S �zerinde"},
    {STRING_HDDINFOUNK6,
    "%6lu %s  S�bit Disk %lu  (Giri�=%hu, Veri Yolu=%hu, Kimlik=%hu)"},
    {STRING_NEWPARTITION,
    "Kur, �zerinde bir yeni b�l�m olu�turdu:"},
    {STRING_UNPSPACE,
    "    %sKullan�lmayan Bo�luk%s           %6lu %s"},
    {STRING_MAXSIZE,
    "MB (En �ok %lu MB)"},
    {STRING_EXTENDED_PARTITION,
    "Geni�letilmi� B�l�m"},
    {STRING_UNFORMATTED,
    "Yeni (Bi�imlendirilmemi�)"},
    {STRING_FORMATUNUSED,
    "Kullan�lmayan"},
    {STRING_FORMATUNKNOWN,
    "Bilinmeyen"},
    {STRING_KB,
    "KB"},
    {STRING_MB,
    "MB"},
    {STRING_GB,
    "GB"},
    {STRING_ADDKBLAYOUTS,
    "D��me tak�m� d�zenleri ekleniyor..."},
    {0, 0}
};
