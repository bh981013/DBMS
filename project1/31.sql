select type
from Pokemon
where id in (select before_id
             from evolution)
group by type
having count(id) >= 3
order by type desc;