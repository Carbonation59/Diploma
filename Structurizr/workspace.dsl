workspace {

    model {
        user = person "User"
        group "Docker" {
            db = softwareSystem "PostgreSQL" {
                db1 = container "Switches" 
                db2 = container "CDRs"  
            }
            poco = softwareSystem "POCO"
        }
        group "Project" {
            gen = softwareSystem "Generator"
            main = softwareSystem "Main Program"
        }
        out = softwareSystem "Output"

        user -> main "Запуск алгоритма"
        main -> gen "Вызов генератора (если требуется)"
        gen -> db1 "Генерация данных"
        gen -> db2 "Генерация данных"
        main -> db1 "Извлечение данных"
        main -> db2 "Извлечение данных"
        poco -> main "Подключение библиотек"
        main -> out "Вывод результата"
    }

    views {
        systemLandscape {
            include *
            autolayout lr
        }
        terminology {

        }
        themes default
    }
    
}