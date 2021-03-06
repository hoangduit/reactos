=========== rosddt - ReactOS device detection tool ===========

Использование:
rosddt -enum  - вывести список всего установленного железа
rosddt -check - проверить совместимость ReactOS с вашей системой
rosddt -update - обновить базу совместимости до последней версии
rosddt -report - отправить отчет об аппаратной конфигурации разработчикам ReactOS. Эта информация будет использована для ведения статистики и поддержки базы совместимости.

=== формат файла rosddt.ini ===
Секция "URL" содержит две ссылки: udpate - ссылка на rosddt.ini, используется при обновлении, report - ссылка на скрипт принимающий отчеты. Всё содержимое отчета передаётся в поле "data" POST запроса в кодировке UTF-8. Пример приёма отчетов смотрите в hw.php.

Секция "HW" содержит базу совместимости ReactOS с железом. Каждое занесённое в базу устройство может иметь три статуса совместимости: ok - устройство работает в ReactOS, notwork - устройство не работает в ReactOS и crash - ReactOS не запускается на системе с этим устройством.
Устройства могут обозначаться следующими способами:
1 - По полному читаемому имени устройства. Например "Intel(R) 82371AB/EB PCI Bus Master IDE Controller".
2 - По полному PNP ID устройства. Например "PCI\VEN_8086&DEV_1237&SUBSYS_00000000&REV_02".
3 - Для PCI устройств можно указывать также сокращенные сокращенные pnp id, такие как "PCI\VEN_8086&DEV_1237" или "PCI\VEN_8086&DEV_1237&REV_02". Рекомендуется пользоваться именно такой формой записи, поскольку полный id привязан к номеру pci слота в который вставлено устройство. Ревизию указывать по необходимости, лучше обойтись без неё.
4 - Для USB устройств можно пользоваться сокращенной формой записи. Например "USB\ROOT_HUB&VID8086&PID3A34&REV0000" можно записать как "USB\VID8086&PID3A34" или "USB\VID8086&PID3A34&REV0000"