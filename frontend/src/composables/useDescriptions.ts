import { Locale } from '@composables/useLanguage';

function useDescriptions() {
    const descriptions = [
        { name: 'player_driver_income', label: Locale('widgets.alerts.player_driver_income'), },
        { name: 'player_job_income', label: Locale('widgets.alerts.player_job_income'), },
        { name: 'player_truck_sold', label: Locale('widgets.alerts.player_truck_sold'), },
        { name: 'player_loan_taken', label: Locale('widgets.alerts.player_loan_taken'), },
        { name: 'player_truck_sold_exchange', label: '', ignore: true },

        { name: 'player_fined', label: Locale('widgets.alerts.player_fined'), },
        { name: 'player_refuel', label: Locale('widgets.alerts.player_refuel'), },
        { name: 'player_emergency_refuel', label: Locale('widgets.alerts.player_emergency_refuel'), },

        { name: 'player_service_fix', label: Locale('widgets.alerts.player_service_fix'), },
        { name: 'player_service_mod', label: Locale('widgets.alerts.player_service_mod'), },
        { name: 'player_service_renovation', label: Locale('widgets.alerts.player_service_renovation'), },
        { name: 'player_service_teleport', label: Locale('widgets.alerts.player_service_teleport'), },
        { name: 'player_service_renovation', label: Locale('widgets.alerts.player_service_renovation'), },

        { name: 'player_job_cancelled', label: Locale('widgets.alerts.player_job_cancelled'), },

        { name: 'player_truck_bought', label: Locale('widgets.alerts.player_truck_bought'), },
        { name: 'player_truck_bought_used_online', label: Locale('widgets.alerts.player_truck_bought_used_online'), },
        
        { name: 'player_garage_teleport', label: Locale('widgets.alerts.player_garage_teleport'), },
        { name: 'player_garage_upgrade', label: Locale('widgets.alerts.player_garage_upgrade'), },
        { name: 'player_garage_bought', label: Locale('widgets.alerts.player_garage_bought'), },
        { name: 'player_garage_bought_online', label: Locale('widgets.alerts.player_garage_bought_online'), },

        { name: 'player_quick_travel', label: Locale('widgets.alerts.player_quick_travel'), },

        { name: 'player_driver_hired', label: Locale('widgets.alerts.player_driver_hired'), },

        { name: 'player_loan_paid', label: Locale('widgets.alerts.player_loan_paid'), },
        { name: 'player_loan_paid_one', label: Locale('widgets.alerts.player_loan_paid_one'), },
        
        { name: 'player_tollgate_paid', label: Locale('widgets.alerts.player_tollgate_paid'), },
    ];

    const types = [
        { name: 'bank', label: Locale('widgets.alerts.bank'), },
        { name: 'xp', label: Locale('widgets.alerts.xp'), },
    ];

    function getLabel(name: any) {
        const description = descriptions.find(desc => desc.name === name);
        return description ? description.label : name;
    }

    function getType(type: any) {
        const typeLabel = types.find(types => types.name === type);
        return typeLabel ? typeLabel.label : type;
    }

    return { descriptions, types, getLabel, getType };
}

export { useDescriptions };