workspace {

    model {
        u = person "User"
        s = softwareSystem "Software System" {
            connect = container "Сервис для удалённого подключения" "" "Программный код"
            arch = container "Удалённый сервер" "" "VS Code Server"{
                main = component "Основной алгоритм"
                gen = component "Генератор"
                database = component "База данных" ""
            }
        }
        
        development = deploymentEnvironment "Development" {
            u -> connect "Запуск алгоритма"
            connect -> arch "Осуществление удалённого соединения"
            main -> database "Чтение информации"
            gen -> database "Заполнение информации"
            main -> gen "Вызов генератора"
            deploymentNode "Visual Studio Code" {
                    containerInstance connect
            }
            deploymentNode "Docker" {
                containerInstance arch
            }
        }

    }

    views {
        deployment * development {
            include * 
            autoLayout lr
        }
        component arch {
            include *
            autoLayout lr
        }
        themes default
    }
}
